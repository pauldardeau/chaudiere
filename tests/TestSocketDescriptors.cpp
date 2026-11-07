// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

// Tests that a Socket or ServerSocket whose construction fails does not leak its
// file descriptor (defect D-6).
//
// A constructor that throws never runs its destructor, so whatever it opened has to
// be released before it throws. Socket(address, port) used to leave the descriptor
// from socket() open when connect() failed, and ServerSocket(port) when bind() failed.
//
// How a leak is detected, without /proc: POSIX gives a new descriptor the lowest
// number that is free, so the number that open() returns is a measure of how many
// descriptors are in use. It is read before and after the failures; every leaked
// descriptor moves it up by one.
//
// Isolation: every check runs in its own forked child process, so that descriptors
// leaked by one cannot disturb another (a leak would otherwise consume the descriptor
// budget of every later check, and the test that lowers RLIMIT_NOFILE must never run
// in the test process itself). The child has no threads. The parent gives each child
// a hard time limit and kills it when it is exceeded, so a regression that blocks can
// not hang the run; the child also has an alarm() backstop of its own.
//
// Ports are chosen by the kernel (bind to port 0), never fixed. The failures are
// made with a port that nothing listens on, with invalid addresses and with a port
// that is taken.

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <chrono>
#include <cstdio>
#include <functional>
#include <string>

#ifdef __linux__
#include <sched.h>
#endif

#include "TestSocketDescriptors.h"
#include "BasicException.h"
#include "ServerSocket.h"
#include "Socket.h"

using namespace chaudiere;

//******************************************************************************

TestSocketDescriptors::TestSocketDescriptors() :
   poivre::TestSuite("TestSocketDescriptors") {
}

//******************************************************************************

namespace {

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
ChildResult runInChild(const std::function<int(FILE* note)>& body, int timeoutMillis) {
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
std::string describe(const char* what, const ChildResult& r) {
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

// ---------------------------------------------------------------- measuring

// POSIX gives a new descriptor the lowest unused number: opening /dev/null and
// closing it again reads off how many descriptors are in use
int lowestFreeDescriptor() {
   const int fd = ::open("/dev/null", O_RDONLY);
   if (fd >= 0) {
      ::close(fd);
   }
   return fd;
}

// a loopback port on which nothing listens: bound to port 0 by the kernel, read back, released
int unusedPort() {
   const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0) {
      return -1;
   }
   sockaddr_in addr;
   ::memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   addr.sin_port = 0;
   socklen_t len = sizeof(addr);
   int port = -1;
   if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0 &&
       ::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) == 0) {
      port = ntohs(addr.sin_port);
   }
   ::close(fd);
   return port;
}

// a listening socket on the given address, port chosen by the kernel
int listenOn(uint32_t address, int backlog, int& portOut) {
   const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0) {
      return -1;
   }
   sockaddr_in addr;
   ::memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(address);
   addr.sin_port = 0;
   socklen_t len = sizeof(addr);
   if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 ||
       ::listen(fd, backlog) != 0 ||
       ::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
      ::close(fd);
      return -1;
   }
   portOut = ntohs(addr.sin_port);
   return fd;
}

// ---------------------------------------------------------------- the checks (run in the child)

// n failed Socket(address, port) constructions must not use up descriptors
int checkFailedSocketsLeakNothing(FILE* note, const std::string& address, int port, int count) {
   // one failure first, so that anything the first use sets up lazily is not counted
   try { Socket warmUp(address, port); } catch (const BasicException&) {}

   const int before = lowestFreeDescriptor();
   int threw = 0;
   int connected = 0;
   for (int i = 0; i < count; ++i) {
      try {
         Socket s(address, port);
         ++connected;
      } catch (const BasicException&) {
         ++threw;
      }
   }
   const int after = lowestFreeDescriptor();

   if (connected != 0) {
      fprintf(note, "%d of %d constructions to '%s':%d unexpectedly connected", connected, count, address.c_str(), port);
      return CHILD_SETUP_FAILED;
   }
   if (threw != count) {
      fprintf(note, "only %d of %d constructions threw", threw, count);
      return CHILD_CHECK_FAILED;
   }
   if (after != before) {
      fprintf(note, "after %d failed Socket(\"%s\", %d) the lowest free descriptor went from %d to %d: %d descriptor(s) leaked",
              count, address.c_str(), port, before, after, after - before);
      return CHILD_CHECK_FAILED;
   }
   fprintf(note, "%d failed constructions, lowest free descriptor stayed at %d", count, before);
   return CHILD_OK;
}

int checkFailedServerSocketsLeakNothing(FILE* note, int count) {
   // a port that is taken: a listener on every interface, as ServerSocket binds
   int takenPort = 0;
   const int holder = listenOn(INADDR_ANY, 1, takenPort);
   if (holder < 0) {
      fprintf(note, "could not occupy a port");
      return CHILD_SETUP_FAILED;
   }

   try { ServerSocket warmUp(takenPort); } catch (const BasicException&) {}

   const int before = lowestFreeDescriptor();
   int threw = 0;
   for (int i = 0; i < count; ++i) {
      try {
         ServerSocket s(takenPort);
      } catch (const BasicException&) {
         ++threw;
      }
   }
   const int after = lowestFreeDescriptor();
   ::close(holder);

   if (threw != count) {
      fprintf(note, "only %d of %d ServerSocket constructions threw", threw, count);
      return CHILD_SETUP_FAILED;
   }
   if (after != before) {
      fprintf(note, "after %d failed ServerSocket(%d) the lowest free descriptor went from %d to %d: %d descriptor(s) leaked",
              count, takenPort, before, after, after - before);
      return CHILD_CHECK_FAILED;
   }
   fprintf(note, "%d failed constructions, lowest free descriptor stayed at %d", count, before);
   return CHILD_OK;
}

// with only a few spare descriptors, repeated failures must never exhaust them
int checkRepeatedFailuresDoNotExhaust(FILE* note, int port, int count, int spare) {
   try { Socket warmUp("127.0.0.1", port); } catch (const BasicException&) {}

   const int base = lowestFreeDescriptor();
   rlimit limit;
   ::getrlimit(RLIMIT_NOFILE, &limit);
   limit.rlim_cur = static_cast<rlim_t>(base + spare);
   if (::setrlimit(RLIMIT_NOFILE, &limit) != 0) {
      fprintf(note, "could not lower RLIMIT_NOFILE: %s", strerror(errno));
      return CHILD_SETUP_FAILED;
   }

   for (int i = 0; i < count; ++i) {
      // can this process still open a descriptor at all?
      const int probe = ::socket(AF_INET, SOCK_STREAM, 0);
      if (probe < 0) {
         fprintf(note, "with %d spare descriptors the process ran out (%s) after %d of %d failed constructions",
                 spare, strerror(errno), i, count);
         return CHILD_CHECK_FAILED;
      }
      ::close(probe);

      try {
         Socket s("127.0.0.1", port);
         fprintf(note, "a construction to a closed port connected");
         return CHILD_SETUP_FAILED;
      } catch (const BasicException&) {
      }
   }

   fprintf(note, "%d failed constructions with %d spare descriptors, none ran out", count, spare);
   return CHILD_OK;
}

// controls: construction that succeeds still works and still releases its descriptor
int checkSuccessfulSocketsStillClose(FILE* note, int count) {
   int port = 0;
   const int listener = listenOn(INADDR_LOOPBACK, 256, port);
   if (listener < 0) {
      fprintf(note, "could not listen");
      return CHILD_SETUP_FAILED;
   }

   { Socket warmUp("127.0.0.1", port); }

   const int before = lowestFreeDescriptor();
   int connectedOk = 0;
   for (int i = 0; i < count; ++i) {
      Socket s("127.0.0.1", port);
      if (s.isConnected() && (s.getFileDescriptor() >= 0) && (s.getPort() == port)) {
         ++connectedOk;
      }
   }
   const int after = lowestFreeDescriptor();
   ::close(listener);

   if (connectedOk != count) {
      fprintf(note, "only %d of %d sockets were connected", connectedOk, count);
      return CHILD_CHECK_FAILED;
   }
   if (after != before) {
      fprintf(note, "lowest free descriptor went from %d to %d after %d successful constructions", before, after, count);
      return CHILD_CHECK_FAILED;
   }
   fprintf(note, "%d connected sockets created and destroyed, lowest free descriptor stayed at %d", count, before);
   return CHILD_OK;
}

int checkSuccessfulServerSocketsStillClose(FILE* note, int count) {
   { ServerSocket warmUp(0); }

   const int before = lowestFreeDescriptor();
   for (int i = 0; i < count; ++i) {
      ServerSocket s(0);                // port 0: the kernel chooses
   }
   const int after = lowestFreeDescriptor();

   if (after != before) {
      fprintf(note, "lowest free descriptor went from %d to %d after %d successful ServerSocket constructions", before, after, count);
      return CHILD_CHECK_FAILED;
   }
   fprintf(note, "%d server sockets created and destroyed, lowest free descriptor stayed at %d", count, before);
   return CHILD_OK;
}

// the exception types and messages that callers see must not change
int checkExceptionsUnchanged(FILE* note, int refusedPort) {
   try {
      Socket s("127.0.0.1", refusedPort);
      fprintf(note, "Socket construction did not throw");
      return CHILD_CHECK_FAILED;
   } catch (const BasicException& be) {
      if (be.whatString() != "Unable to open socket") {
         fprintf(note, "Socket threw BasicException '%s'", be.whatString().c_str());
         return CHILD_CHECK_FAILED;
      }
   } catch (...) {
      fprintf(note, "Socket threw something other than BasicException");
      return CHILD_CHECK_FAILED;
   }

   int takenPort = 0;
   const int holder = listenOn(INADDR_ANY, 1, takenPort);
   if (holder < 0) {
      fprintf(note, "could not occupy a port");
      return CHILD_SETUP_FAILED;
   }
   int result = CHILD_OK;
   try {
      ServerSocket s(takenPort);
      fprintf(note, "ServerSocket construction did not throw");
      result = CHILD_CHECK_FAILED;
   } catch (const BasicException& be) {
      if (be.whatString() != "unable to create server socket") {
         fprintf(note, "ServerSocket threw BasicException '%s'", be.whatString().c_str());
         result = CHILD_CHECK_FAILED;
      }
   } catch (...) {
      fprintf(note, "ServerSocket threw something other than BasicException");
      result = CHILD_CHECK_FAILED;
   }
   ::close(holder);
   if (result == CHILD_OK) {
      fprintf(note, "Socket: BasicException 'Unable to open socket'; ServerSocket: BasicException 'unable to create server socket'");
   }
   return result;
}

const int TIME_LIMIT_MILLIS = 20000;

}

//******************************************************************************
// failed Socket construction

// a child's exit code is either "met" or, for the one environment-dependent check, "skipped"
static bool passed(const ChildResult& r) {
   return r.exited && ((r.exitCode == CHILD_OK) || (r.exitCode == CHILD_SKIPPED));
}

POIVRE_TEST_CASE(TestSocketDescriptors, testFailedConnectionRefusedLeaksNoDescriptor) {
   const int port = unusedPort();
   require(port > 0, "a port that nothing listens on could be chosen");

   const ChildResult r = runInChild([port](FILE* note) {
      return checkFailedSocketsLeakNothing(note, "127.0.0.1", port, 50);
   }, TIME_LIMIT_MILLIS);

   require(passed(r), describe("connection refused", r));
}

POIVRE_TEST_CASE(TestSocketDescriptors, testFailedConnectionInvalidAddressLeaksNoDescriptor) {
   // all of these reach connect() as 255.255.255.255 (inet_addr cannot report an
   // error); each runs in a child of its own
   const char* addresses[] = { "not-an-address", "", "999.999.999.999", "256.0.0.1", "localhost", "::1" };

   for (const char* address : addresses) {
      const std::string a = address;
      const ChildResult r = runInChild([a](FILE* note) {
         return checkFailedSocketsLeakNothing(note, a, 80, 30);
      }, TIME_LIMIT_MILLIS);

      require(passed(r), describe(("invalid address '" + a + "'").c_str(), r));
   }
}

POIVRE_TEST_CASE(TestSocketDescriptors, testFailedConnectionToOddPortsLeaksNoDescriptor) {
   // port 0, and ports that wrap around in htons(); each runs in a child of its own
   const int ports[] = { 0, 65536, -1 };

   for (int port : ports) {
      const ChildResult r = runInChild([port](FILE* note) {
         return checkFailedSocketsLeakNothing(note, "127.0.0.1", port, 30);
      }, TIME_LIMIT_MILLIS);

      require(passed(r), describe(("port " + std::to_string(port)).c_str(), r));
   }
}

POIVRE_TEST_CASE(TestSocketDescriptors, testFailedConnectionUnreachableLeaksNoDescriptor) {
   // "network unreachable" needs a destination without a route. A child in an empty network
   // namespace has none (Linux, when unprivileged user namespaces are allowed; otherwise the
   // check is skipped and says so). Without it, connect() to a non-routable address sits in
   // SYN retries for minutes, because Socket has no connect timeout.
   const ChildResult r = runInChild([](FILE* note) {
#ifdef __linux__
      if (::unshare(CLONE_NEWUSER | CLONE_NEWNET) != 0) {
         fprintf(note, "skipped: cannot create a network namespace (%s)", strerror(errno));
         return CHILD_SKIPPED;
      }
      return checkFailedSocketsLeakNothing(note, "10.255.255.1", 80, 30);
#else
      fprintf(note, "skipped: no network namespaces on this platform");
      return CHILD_SKIPPED;
#endif
   }, TIME_LIMIT_MILLIS);

   require(passed(r), describe("network unreachable", r));
   if (r.exited && (r.exitCode == CHILD_SKIPPED)) {
      printf("   note: %s\n", r.diagnostics.c_str());
      fflush(stdout);
   }
}

//******************************************************************************
// repeated failures

POIVRE_TEST_CASE(TestSocketDescriptors, testRepeatedFailuresDoNotAccumulateDescriptors) {
   const int port = unusedPort();
   require(port > 0, "a port that nothing listens on could be chosen");

   const ChildResult r = runInChild([port](FILE* note) {
      return checkFailedSocketsLeakNothing(note, "127.0.0.1", port, 300);
   }, TIME_LIMIT_MILLIS);

   require(passed(r), describe("300 repeated failures", r));
}

POIVRE_TEST_CASE(TestSocketDescriptors, testRepeatedFailuresDoNotExhaustDescriptors) {
   // the child lowers RLIMIT_NOFILE to 16 more than it uses, then fails 200 times
   const int port = unusedPort();
   require(port > 0, "a port that nothing listens on could be chosen");

   const ChildResult r = runInChild([port](FILE* note) {
      return checkRepeatedFailuresDoNotExhaust(note, port, 200, 16);
   }, TIME_LIMIT_MILLIS);

   require(passed(r), describe("descriptor exhaustion under a reduced RLIMIT_NOFILE", r));
}

//******************************************************************************
// failed ServerSocket construction

POIVRE_TEST_CASE(TestSocketDescriptors, testFailedServerSocketLeaksNoDescriptor) {
   const ChildResult r = runInChild([](FILE* note) {
      return checkFailedServerSocketsLeakNothing(note, 50);
   }, TIME_LIMIT_MILLIS);

   require(passed(r), describe("ServerSocket on a port in use", r));
}

//******************************************************************************
// controls: what must keep working

POIVRE_TEST_CASE(TestSocketDescriptors, testSuccessfulSocketStillConnectsAndReleasesItsDescriptor) {
   const ChildResult r = runInChild([](FILE* note) {
      return checkSuccessfulSocketsStillClose(note, 50);
   }, TIME_LIMIT_MILLIS);

   require(passed(r), describe("successful Socket construction", r));
}

POIVRE_TEST_CASE(TestSocketDescriptors, testSuccessfulServerSocketStillReleasesItsDescriptor) {
   const ChildResult r = runInChild([](FILE* note) {
      return checkSuccessfulServerSocketsStillClose(note, 50);
   }, TIME_LIMIT_MILLIS);

   require(passed(r), describe("successful ServerSocket construction", r));
}

POIVRE_TEST_CASE(TestSocketDescriptors, testFailedConstructionsStillThrowTheSameExceptions) {
   const int port = unusedPort();
   require(port > 0, "a port that nothing listens on could be chosen");

   const ChildResult r = runInChild([port](FILE* note) {
      return checkExceptionsUnchanged(note, port);
   }, TIME_LIMIT_MILLIS);

   require(passed(r), describe("exception types and messages", r));
}
