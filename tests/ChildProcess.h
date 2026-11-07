// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTS_CHILDPROCESS_H
#define CHAUDIERE_TESTS_CHILDPROCESS_H

// Runs a check in a forked child process, with a hard time limit. POSIX only.
//
// Used by the tests that must not share a process with what they exercise: a check that
// leaks descriptors, lowers RLIMIT_NOFILE or leaves SIGPIPE at its default disposition.
// The child has no threads. The parent kills a child that exceeds the limit, and the child
// has an alarm() backstop of its own, so a regression that blocks cannot hang the run.
// The child reports a one-line reason through a pipe (see the FILE* note of the body), and
// describe() puts it into the text of a failed require().

#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <functional>
#include <string>

namespace chaudiere_test
{

typedef std::chrono::steady_clock Clock;

// ---------------------------------------------------------------- isolation

// exit codes of a child (anything else is a failure whose reason is in the diagnostics)
const int CHILD_OK = 0;
const int CHILD_CHECK_FAILED = 1;
const int CHILD_SETUP_FAILED = 2;
const int CHILD_SKIPPED = 77;

struct ChildResult {
   bool exited = false;          // exited normally (not killed by a signal, not timed out)
   int exitCode = -1;
   int signalNumber = 0;
   bool timedOut = false;
   std::string diagnostics;      // what the child wrote with note()
};

// runs body in a forked child and waits for it, for at most timeoutMillis
inline ChildResult runInChild(const std::function<int(FILE* note)>& body, int timeoutMillis) {
   ChildResult result;

   int pipeFds[2];
   if (::pipe(pipeFds) != 0) {
      result.diagnostics = "pipe() failed";
      return result;
   }

   ::fflush(stdout);
   ::fflush(stderr);

   const pid_t pid = ::fork();
   if (pid < 0) {
      ::close(pipeFds[0]);
      ::close(pipeFds[1]);
      result.diagnostics = "fork() failed";
      return result;
   }

   if (pid == 0) {
      // child: nothing it prints to stdout (ServerSocket's bind() reports errno
      // there) is of interest, and it must not outlive its time limit
      ::close(pipeFds[0]);
      ::alarm(static_cast<unsigned int>(timeoutMillis / 1000 + 5));
      if (::freopen("/dev/null", "w", stdout) == nullptr) {
         ::_exit(CHILD_SETUP_FAILED);
      }
      FILE* note = ::fdopen(pipeFds[1], "w");
      if (note == nullptr) {
         ::_exit(CHILD_SETUP_FAILED);
      }
      const int code = body(note);
      ::fflush(note);
      ::_exit(code);
   }

   ::close(pipeFds[1]);

   const Clock::time_point start = Clock::now();
   bool eof = false;
   int status = 0;
   bool reaped = false;

   while (!reaped) {
      const long long elapsed =
         std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count();
      if (elapsed >= timeoutMillis) {
         result.timedOut = true;
         ::kill(pid, SIGKILL);
         ::waitpid(pid, &status, 0);
         reaped = true;
         break;
      }

      if (!eof) {
         pollfd p;
         p.fd = pipeFds[0];
         p.events = POLLIN;
         p.revents = 0;
         if (::poll(&p, 1, 50) > 0) {
            char buffer[512];
            const ssize_t n = ::read(pipeFds[0], buffer, sizeof(buffer));
            if (n > 0) {
               result.diagnostics.append(buffer, static_cast<size_t>(n));
            } else if (n == 0) {
               eof = true;
            }
         }
      } else {
         ::usleep(5000);
      }

      if (::waitpid(pid, &status, WNOHANG) == pid) {
         reaped = true;
      }
   }

   // anything the child wrote just before it ended
   {
      pollfd p;
      p.fd = pipeFds[0];
      p.events = POLLIN;
      p.revents = 0;
      char buffer[512];
      while (::poll(&p, 1, 0) > 0) {
         const ssize_t n = ::read(pipeFds[0], buffer, sizeof(buffer));
         if (n <= 0) {
            break;
         }
         result.diagnostics.append(buffer, static_cast<size_t>(n));
      }
   }
   ::close(pipeFds[0]);

   if (!result.timedOut) {
      if (WIFEXITED(status)) {
         result.exited = true;
         result.exitCode = WEXITSTATUS(status);
      } else if (WIFSIGNALED(status)) {
         result.signalNumber = WTERMSIG(status);
      }
   }

   return result;
}

// the text of a failed check, for require()
inline std::string describe(const char* what, const ChildResult& r) {
   std::string text = what;
   if (r.timedOut) {
      text += ": the child did not finish in time and was killed";
   } else if (!r.exited) {
      text += ": the child was terminated by signal " + std::to_string(r.signalNumber);
   } else if (r.exitCode == CHILD_SETUP_FAILED) {
      text += ": the child could not set the test up";
   } else if (r.exitCode != CHILD_OK) {
      text += ": check failed";
   }
   if (!r.diagnostics.empty()) {
      text += " [" + r.diagnostics + "]";
   }
   return text;
}

}

#endif
