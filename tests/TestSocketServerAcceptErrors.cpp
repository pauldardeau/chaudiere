// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

// Tests of what SocketServer::runSocketServer() does when accept() fails (defect F-07).
//
// The loop used to retry every failure at once: a failure that does not go away (a listening
// descriptor that has become invalid, a full descriptor table) made it fail millions of times a
// second at 100% CPU and say nothing. It now classifies the error, pauses between retries with a
// bounded exponential back-off, ends with a non-zero result when the listener is unusable, and
// reports a failing streak without flooding the log (src/internal/AcceptPolicy.h has the policy).
//
// Three kinds of test:
//
//   1. Policy tests call the classifier and the failure tracker that the server uses, directly, with
//      no sockets and no clock: every errno, the pause sequence, the guard, the resets, the logging
//      rate limit.
//   2. Scripted loop tests run the REAL SocketServer::run() (real listener, real handlers) with
//      ServerSocket::accept() taking its result from a script (chaudiere::detail::setAcceptIoOps)
//      and with a virtual clock and a recording pause (setAcceptLoopOps): errors the kernel will not
//      produce on demand are injected, a "success" step performs the real accept() of a client the
//      test has queued. The script ends with a terminal error, which is what ends the loop, so
//      every loop runs to its natural end and returns. These are INJECTED results; the tests say so.
//   3. Integration tests run the real server against the real kernel: a listening descriptor that
//      has been closed, real descriptor exhaustion (a lowered RLIMIT_NOFILE in the test's own child
//      process, a client in a separate process so that it has its own descriptor table) and a real
//      signal that makes the blocked accept() return EINTR. A signal that was merely delivered
//      proves nothing: the EINTR test passes only if accept() really returned -1/EINTR, and reports
//      INCONCLUSIVE otherwise. INCONCLUSIVE is printed as such and never counted as a pass.
//
// Isolation and limits: every loop runs in a forked child (ChildProcess.h) with a hard time limit,
// so a regression that spins or blocks cannot hang the run, and a child that lowers its descriptor
// limit cannot affect the test executable. Loopback only, ports chosen by the kernel, no root.
// No test here does anything to the whole system: ENFILE, ENOBUFS and ENOMEM are only injected.

#include <atomic>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "TestSocketServerAcceptErrors.h"
#include "ChildProcess.h"
#include "Logger.h"
#include "RequestHandler.h"
#include "Socket.h"
#include "SocketServer.h"
#include "SocketServiceHandler.h"
#include "internal/AcceptPolicy.h"
#include "internal/SocketIo.h"

using namespace chaudiere;
using namespace chaudiere_test;

//******************************************************************************

TestSocketServerAcceptErrors::TestSocketServerAcceptErrors() :
   poivre::TestSuite("TestSocketServerAcceptErrors") {
}

//******************************************************************************

namespace {

const int CHILD_INCONCLUSIVE = CHILD_SKIPPED;
const int TIME_LIMIT_MILLIS = 40000;

long long nowMs() {
   return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
}

void sleepMs(long long ms) {
   std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

double processCpuSeconds() {
   struct timespec ts;
   ::clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
   return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) / 1e9;
}

// what a check collects about itself; a failed check() does not stop the child, so one run reports every failure
FILE* g_note = nullptr;      // the note of the check that is running (used to say why a runaway loop was stopped)

class Report {
public:
   explicit Report(FILE* note) : m_note(note), m_ok(true) { g_note = note; }

   void check(bool condition, const char* format, ...) {
      if (condition) {
         return;
      }
      m_ok = false;
      char text[400];
      va_list args;
      va_start(args, format);
      ::vsnprintf(text, sizeof(text), format, args);
      va_end(args);
      ::fprintf(m_note, "FAILED: %s; ", text);
   }

   void fact(const char* format, ...) {
      char text[400];
      va_list args;
      va_start(args, format);
      ::vsnprintf(text, sizeof(text), format, args);
      va_end(args);
      ::fprintf(m_note, "%s; ", text);
   }

   int result() const { return m_ok ? CHILD_OK : CHILD_CHECK_FAILED; }

private:
   FILE* m_note;
   bool m_ok;
};

//******************************************************************************
// a logger that keeps what it is given

struct LogEntry {
   LogLevel level;
   std::string text;
};

class CapturingLogger : public Logger {
public:
   explicit CapturingLogger(bool clobberErrno = false) : m_clobberErrno(clobberErrno) {}

   LogLevel getLogLevel() const override { return Verbose; }
   void setLogLevel(LogLevel) override {}
   bool isLoggingInstanceLifecycles() const override { return false; }
   void setLogInstanceLifecycles(bool) override {}
   void logInstanceCreate(const std::string&) override {}
   void logInstanceDestroy(const std::string&) override {}
   bool isLoggingLevel(LogLevel) const override { return true; }
   void logOccurrence(const std::string&, const std::string&) override {}

   void logMessage(LogLevel level, const std::string& message) override {
      if (m_clobberErrno) {
         errno = EBADF;      // a logger is free to do anything to errno
      }
      std::lock_guard<std::mutex> lock(m_mutex);
      LogEntry entry;
      entry.level = level;
      entry.text = message;
      m_entries.push_back(entry);
   }

   std::vector<LogEntry> entries() {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_entries;
   }

   int count(LogLevel level) {
      int n = 0;
      for (const LogEntry& e : entries()) {
         if (e.level == level) {
            ++n;
         }
      }
      return n;
   }

   std::string first(LogLevel level) {
      for (const LogEntry& e : entries()) {
         if (e.level == level) {
            return e.text;
         }
      }
      return std::string();
   }

private:
   std::mutex m_mutex;
   std::vector<LogEntry> m_entries;
   bool m_clobberErrno;
};

// installs a CapturingLogger for the life of the guard; the Logger class owns what it is given
class LoggerScope {
public:
   explicit LoggerScope(bool clobberErrno = false) : m_logger(new CapturingLogger(clobberErrno)) {
      Logger::setLogger(m_logger);
   }
   ~LoggerScope() { Logger::setLogger(nullptr); }
   CapturingLogger* operator->() { return m_logger; }
   LoggerScope(const LoggerScope&) = delete;
   LoggerScope& operator=(const LoggerScope&) = delete;
private:
   CapturingLogger* m_logger;   // owned by Logger while installed
};

//******************************************************************************
// the server under test: real listener, real handlers (echo "ping")

class EchoRequestHandler : public RequestHandler {
public:
   explicit EchoRequestHandler(Socket* socket) : RequestHandler(socket) {}
   explicit EchoRequestHandler(SocketRequest* socketRequest) : RequestHandler(socketRequest) {}

   void run() override {
      Socket* socket = getSocket();
      if (nullptr != socket) {
         char buffer[4];
         if (socket->readSocket(buffer, 4, 3000) == 4) {
            socket->write(buffer, 4);
         }
      }
   }
};

class EchoSocketServiceHandler : public SocketServiceHandler {
public:
   void serviceSocket(SocketRequest* socketRequest) override {
      EchoRequestHandler handler(socketRequest);
      handler.run();
   }
   const std::string& getName() const override {
      static const std::string name = "EchoSocketServiceHandler";
      return name;
   }
};

class AcceptTestServer : public SocketServer {
public:
   AcceptTestServer(const std::string& configFilePath) :
      SocketServer("AcceptTestServer", "0.1", configFilePath) {
   }
   RequestHandler* handlerForSocket(Socket* socket) override {
      return new EchoRequestHandler(socket);
   }
   RequestHandler* handlerForSocketRequest(SocketRequest* socketRequest) override {
      return new EchoRequestHandler(socketRequest);
   }
   SocketServiceHandler* createSocketServiceHandler() override {
      return new EchoSocketServiceHandler();
   }
};

// a port that was free a moment ago (the server's configuration can only name a port)
int freePort() {
   const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0) {
      return -1;
   }
   sockaddr_in addr;
   ::memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   addr.sin_port = 0;
   int port = -1;
   socklen_t length = sizeof(addr);
   if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0 &&
       ::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &length) == 0) {
      port = ntohs(addr.sin_port);
   }
   ::close(fd);
   return port;
}

// a configuration file for a server on `port`; "" if it cannot be written
std::string writeConfig(int port, const char* threading) {
   char path[] = "/tmp/chaudiere_accept_errors_XXXXXX";
   const int fd = ::mkstemp(path);
   if (fd < 0) {
      return std::string();
   }
   char text[200];
   const int n = ::snprintf(text, sizeof(text), "[server]\nport = %d\nthreading = %s\n", port, threading);
   const bool ok = (::write(fd, text, static_cast<size_t>(n)) == n);
   ::close(fd);
   if (!ok) {
      ::unlink(path);
      return std::string();
   }
   return std::string(path);
}

bool readableWithin(int fd, long long millis) {
   const long long limit = nowMs() + millis;
   for (;;) {
      const long long left = limit - nowMs();
      if (left <= 0) {
         return false;
      }
      pollfd p;
      p.fd = fd;
      p.events = POLLIN;
      p.revents = 0;
      const int n = ::poll(&p, 1, static_cast<int>(left));
      if (n > 0) {
         return true;
      }
      if (n < 0 && errno != EINTR) {
         return false;
      }
   }
}

// a blocking client connected to the loopback port; -1 if the connection fails
int connectClient(int port) {
   const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0) {
      return -1;
   }
   sockaddr_in addr;
   ::memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   addr.sin_port = htons(static_cast<unsigned short>(port));
   if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
      ::close(fd);
      return -1;
   }
   return fd;
}

// the client side of the echo exchange: waits for "ping" back
bool echoComes(int fd, long long millis) {
   char buffer[4];
   size_t got = 0;
   const long long limit = nowMs() + millis;
   while (got < 4) {
      const long long left = limit - nowMs();
      if (left <= 0 || !readableWithin(fd, left)) {
         return false;
      }
      const ssize_t n = ::recv(fd, buffer + got, 4 - got, 0);
      if (n <= 0) {
         return false;
      }
      got += static_cast<size_t>(n);
   }
   return ::memcmp(buffer, "ping", 4) == 0;
}

// a complete exchange with the server: connect, "ping", wait for the echo
bool pingOnce(int port, long long millis) {
   const int fd = connectClient(port);
   if (fd < 0) {
      return false;
   }
   const bool sent = (::send(fd, "ping", 4, 0) == 4);
   const bool ok = sent && echoComes(fd, millis);
   ::close(fd);
   return ok;
}

//******************************************************************************
// the scripted accept() and the virtual clock (INJECTED results)

enum StepKind {
   STEP_FAIL,       // -1 with errno = error, nothing is accepted
   STEP_REAL        // the real accept(): a client the test queued is accepted
};

struct Step {
   StepKind kind;
   int error;
};

// builds a script
class Steps {
public:
   Steps& fail(int error, int times = 1) {
      for (int i = 0; i < times; ++i) {
         Step s = { STEP_FAIL, error };
         m_steps.push_back(s);
      }
      return *this;
   }
   Steps& real(int times = 1) {
      for (int i = 0; i < times; ++i) {
         Step s = { STEP_REAL, 0 };
         m_steps.push_back(s);
      }
      return *this;
   }
   const std::vector<Step>& steps() const { return m_steps; }
private:
   std::vector<Step> m_steps;
};

struct LoopScript {
   std::vector<Step> steps;
   size_t next = 0;
   int endError = EBADF;                 // what a call after the last step returns: ends the loop
   long calls = 0;
   std::vector<long> sleepsAtCall;       // the number of pauses made before each accept() call
   std::vector<int> pauses;              // every pause the loop asked for, in milliseconds
   long long virtualNowMs = 1000000;
};

LoopScript* g_script = nullptr;          // used only by the one thread that runs the loop

int scriptedAccept(int fd, sockaddr* address, socklen_t* addressLength) {
   LoopScript* script = g_script;
   ++script->calls;
   if (script->calls > 20000) {
      // no script here is anywhere near this long: a loop that keeps calling accept() after the terminal
      // error at the end of its script is a regression, and must not be left to fill memory until the
      // child's time limit
      if (g_note != nullptr) {
         ::fprintf(g_note, "FAILED: the loop called accept() more than 20000 times for a script of %zu steps (it does not end); ", script->steps.size());
         ::fflush(g_note);
      }
      ::_exit(CHILD_CHECK_FAILED);
   }
   script->sleepsAtCall.push_back(static_cast<long>(script->pauses.size()));
   Step step;
   if (script->next < script->steps.size()) {
      step = script->steps[script->next++];
   } else {
      step.kind = STEP_FAIL;
      step.error = script->endError;
   }
   if (step.kind == STEP_REAL) {
      return ::accept(fd, address, addressLength);
   }
   errno = step.error;
   return -1;
}

std::chrono::steady_clock::time_point virtualNow() {
   return std::chrono::steady_clock::time_point(std::chrono::milliseconds(g_script->virtualNowMs));
}

void virtualSleep(int millis) {
   g_script->pauses.push_back(millis);
   g_script->virtualNowMs += millis;
}

const detail::AcceptIoOps SCRIPTED_ACCEPT_OPS = { scriptedAccept };
const detail::AcceptLoopOps VIRTUAL_LOOP_OPS = { virtualNow, virtualSleep };

// what the real loop did with a script
struct LoopOutcome {
   bool setupOk = false;
   int rc = -99;
   long calls = 0;
   std::vector<int> pauses;
   std::vector<long> sleepsAtCall;
   long long virtualElapsedMs = 0;
   int clients = 0;
   int echoed = 0;
   int critical = 0;
   int warnings = 0;
   int infos = 0;
   std::string firstCritical;
   std::string firstWarning;
   std::string firstInfo;
   size_t scriptStepsUsed = 0;
};

// Runs the REAL SocketServer::run() in the calling process: a real listener, `clients` real client
// connections already queued on it (each has sent "ping", which the real handler echoes), accept()
// results from the script, a virtual clock and a recording pause. After the last step every accept()
// returns endError, which is terminal by default and so ends the loop. The caller is a forked child.
LoopOutcome runScripted(const Steps& steps, int clients, const char* threading,
                        int endError = EBADF, bool clobberErrnoInLogger = false) {
   LoopOutcome outcome;
   LoggerScope logger(clobberErrnoInLogger);

   const int port = freePort();
   const std::string config = writeConfig(port, threading);
   if (port < 0 || config.empty()) {
      return outcome;
   }

   LoopScript script;
   script.steps = steps.steps();
   script.endError = endError;
   g_script = &script;

   std::vector<int> clientFds;
   {
      AcceptTestServer server(config);
      for (int i = 0; i < clients; ++i) {
         const int fd = connectClient(port);
         if (fd < 0) {
            break;
         }
         ::send(fd, "ping", 4, 0);
         clientFds.push_back(fd);
      }

      if (static_cast<int>(clientFds.size()) == clients) {
         const detail::AcceptIoOps* previousAccept = detail::setAcceptIoOps(&SCRIPTED_ACCEPT_OPS);
         const detail::AcceptLoopOps* previousLoop = detail::setAcceptLoopOps(&VIRTUAL_LOOP_OPS);
         outcome.rc = server.run();
         detail::setAcceptIoOps(previousAccept);
         detail::setAcceptLoopOps(previousLoop);
         outcome.setupOk = true;
      }

      for (int fd : clientFds) {
         if (echoComes(fd, 4000)) {
            ++outcome.echoed;
         }
      }
   }   // the server (and with it a thread pool) is destroyed here

   for (int fd : clientFds) {
      ::close(fd);
   }
   ::unlink(config.c_str());

   outcome.clients = clients;
   outcome.calls = script.calls;
   outcome.pauses = script.pauses;
   outcome.sleepsAtCall = script.sleepsAtCall;
   outcome.virtualElapsedMs = script.virtualNowMs - 1000000;
   outcome.scriptStepsUsed = script.next;
   outcome.critical = logger->count(Critical);
   outcome.warnings = logger->count(Warning);
   outcome.infos = logger->count(Info);
   outcome.firstCritical = logger->first(Critical);
   outcome.firstWarning = logger->first(Warning);
   outcome.firstInfo = logger->first(Info);
   g_script = nullptr;
   return outcome;
}

// 10, 20, 40, 80, 160, 250, 250, ...
std::vector<int> expectedPauses(size_t n) {
   std::vector<int> pauses;
   int pause = 10;
   for (size_t i = 0; i < n; ++i) {
      pauses.push_back(pause);
      pause = (pause * 2 > 250) ? 250 : pause * 2;
   }
   return pauses;
}

std::string listOf(const std::vector<int>& v, size_t limit = 12) {
   std::string s;
   for (size_t i = 0; i < v.size() && i < limit; ++i) {
      s += (i ? "," : "") + std::to_string(v[i]);
   }
   if (v.size() > limit) {
      s += ",...";
   }
   return s;
}

std::string errnoText(int error) {
   return "errno " + std::to_string(error);
}

//******************************************************************************
// scripted loop checks (each runs in a forked child)

// T1: EINTR, EINTR, success: retried at once, no pause, dispatched, and the state resets
int checkImmediateRetry(FILE* note) {
   Report r(note);
   // 99 EINTR twice, with a success between: never the 101st in a row, so never a pause. The
   // first block is followed by the success, which must reset the count.
   Steps steps;
   steps.fail(EINTR, 2).real().fail(EINTR, 99).real().fail(EINTR, 99).real();
   const LoopOutcome o = runScripted(steps, 3, "none");
   r.check(o.setupOk, "the server could not be set up");
   r.check(o.rc == 1, "run() returned %d, expected 1 (the loop ends with the terminal error after the script)", o.rc);
   r.check(o.pauses.empty(), "the loop paused (%s) although no failure was past the immediate-retry limit", listOf(o.pauses).c_str());
   r.check(o.calls == 2 + 1 + 99 + 1 + 99 + 1 + 1, "%ld accept() calls, expected 203", o.calls);
   r.check(o.echoed == 3, "%d of 3 accepted connections were handled (echoed)", o.echoed);
   r.check(o.warnings == 0 && o.infos == 0, "failures that were retried at once were reported (%d warnings, %d infos)", o.warnings, o.infos);
   r.check(o.critical == 1, "%d critical diagnostics, expected 1 (the end of the script)", o.critical);
   r.fact("%ld accept() calls, %d pauses, %d of 3 connections echoed", o.calls, static_cast<int>(o.pauses.size()), o.echoed);
   return r.result();
}

// T2: more than 100 EINTR in a row: the guard activates, the loop pauses, the call count is bounded
int checkImmediateRetryGuard(FILE* note) {
   Report r(note);
   const int FAILURES = 250;
   Steps steps;
   steps.fail(EINTR, FAILURES);
   const LoopOutcome o = runScripted(steps, 0, "none");
   r.check(o.setupOk, "the server could not be set up");
   r.check(o.rc == 1, "run() returned %d, expected 1", o.rc);
   r.check(o.calls == FAILURES + 1, "%ld accept() calls, expected %d", o.calls, FAILURES + 1);
   // failures 1..100 are retried at once; the 101st and every later one is followed by a pause
   r.check(static_cast<int>(o.pauses.size()) == FAILURES - 100, "%d pauses, expected %d", static_cast<int>(o.pauses.size()), FAILURES - 100);
   bool shape = o.sleepsAtCall.size() == static_cast<size_t>(FAILURES + 1);
   for (int call = 1; shape && call <= FAILURES + 1; ++call) {
      const long expected = (call > 101) ? (call - 101) : 0;
      shape = (o.sleepsAtCall[static_cast<size_t>(call - 1)] == expected);
   }
   r.check(shape, "the pauses did not begin right after the 100th immediate retry (pauses before each call differ from 0 up to call 101, then 1, 2, 3, ...)");
   r.check(o.pauses == expectedPauses(o.pauses.size()), "the pauses were %s, expected 10,20,40,80,160,250,...", listOf(o.pauses).c_str());
   // the streak begins when the guard activates: one warning for it, then at most one per 5 s of (virtual) time
   const long long maxWarnings = 1 + o.virtualElapsedMs / 5000;
   r.check(o.warnings >= 1 && o.warnings <= maxWarnings, "%d warnings in %lld virtual ms, expected 1 to %lld", o.warnings, o.virtualElapsedMs, maxWarnings);
   r.fact("%ld accept() calls, %d pauses [%s], %d warning", o.calls, static_cast<int>(o.pauses.size()), listOf(o.pauses).c_str(), o.warnings);
   return r.result();
}

// T3: persistent EMFILE: 10, 20, 40, 80, 160, 250, 250, ...
int checkExponentialBackoff(FILE* note) {
   Report r(note);
   const int FAILURES = 12;
   Steps steps;
   steps.fail(EMFILE, FAILURES);
   const LoopOutcome o = runScripted(steps, 0, "none");
   r.check(o.setupOk, "the server could not be set up");
   r.check(o.rc == 1, "run() returned %d, expected 1", o.rc);
   r.check(o.calls == FAILURES + 1, "%ld accept() calls, expected %d", o.calls, FAILURES + 1);
   r.check(o.pauses == expectedPauses(FAILURES), "the pauses were %s, expected 10,20,40,80,160,250,250,...", listOf(o.pauses).c_str());
   r.check(o.pauses.size() == static_cast<size_t>(FAILURES), "%d pauses", static_cast<int>(o.pauses.size()));
   r.fact("pauses %s", listOf(o.pauses).c_str());
   return r.result();
}

// T4: EMFILE, EMFILE, success, EMFILE: the pause starts again at 10 ms
int checkBackoffResetsAfterSuccess(FILE* note) {
   Report r(note);
   Steps steps;
   steps.fail(EMFILE, 2).real().fail(EMFILE, 2);
   const LoopOutcome o = runScripted(steps, 1, "none");
   r.check(o.setupOk, "the server could not be set up");
   const std::vector<int> expected = { 10, 20, 10, 20 };
   r.check(o.pauses == expected, "the pauses were %s, expected 10,20,10,20", listOf(o.pauses).c_str());
   r.check(o.echoed == 1, "the accepted connection was not handled");
   r.check(o.infos == 1, "%d recovery messages, expected 1", o.infos);
   r.fact("pauses %s", listOf(o.pauses).c_str());
   return r.result();
}

// T5: alternating errors do not restart the back-off, and immediate-retry errors in between are not a reset
int checkAlternatingErrors(FILE* note) {
   Report r(note);
   Steps steps;
   steps.fail(EMFILE).fail(ENOMEM).fail(EMFILE).fail(EAGAIN).fail(ENFILE).fail(ENOBUFS).fail(EMFILE).fail(ENOMEM);
   steps.fail(EINTR).fail(EMFILE).fail(ECONNABORTED).fail(ENOMEM);
   const LoopOutcome o = runScripted(steps, 0, "none");
   r.check(o.setupOk, "the server could not be set up");
   // EINTR and ECONNABORTED are retried at once (no pause, not a reset): 10 pauses for the 10 others
   r.check(o.pauses == expectedPauses(10), "the pauses were %s, expected 10,20,40,80,160,250,250,250,250,250", listOf(o.pauses, 20).c_str());
   r.fact("pauses %s", listOf(o.pauses, 20).c_str());
   return r.result();
}

// T6: every terminal error ends the loop at once, with a non-zero result and one critical diagnostic
int checkTerminalError(int error, FILE* note) {
   Report r(note);
   Steps steps;
   steps.fail(EINTR, 2).fail(error);       // two harmless interruptions first, then the terminal error
   const LoopOutcome o = runScripted(steps, 0, "none", EMFILE /* would be retried: must not be reached */);
   r.check(o.setupOk, "the server could not be set up");
   r.check(o.rc != 0, "run() returned 0 after a terminal error");
   r.check(o.rc == 1, "run() returned %d, expected 1", o.rc);
   r.check(o.calls == 3, "%ld accept() calls, expected 3 (no retry after the terminal error)", o.calls);
   r.check(o.pauses.empty(), "the loop paused before ending");
   r.check(o.critical == 1, "%d critical diagnostics, expected 1", o.critical);
   r.check(o.firstCritical.find(errnoText(error)) != std::string::npos, "the critical diagnostic does not name %s: '%s'", errnoText(error).c_str(), o.firstCritical.c_str());
   r.check(o.warnings == 0 && o.infos == 0, "unexpected warnings/infos (%d/%d)", o.warnings, o.infos);
   r.fact("rc %d after %ld calls; critical: %s", o.rc, o.calls, o.firstCritical.c_str());
   return r.result();
}

// T7: errors outside every list: bounded back-off, never a tight loop, never terminal
int checkUnknownErrors(FILE* note) {
   Report r(note);
   Steps steps;
   steps.fail(EIO).fail(EPERM).fail(ENXIO).fail(9999).fail(EOPNOTSUPP).fail(ETIMEDOUT).fail(EIO).fail(EPERM);
   const LoopOutcome o = runScripted(steps, 0, "none");
   r.check(o.setupOk, "the server could not be set up");
   r.check(o.calls == 9, "%ld accept() calls, expected 9 (8 failures, then the end of the script)", o.calls);
   r.check(o.pauses == expectedPauses(8), "the pauses were %s, expected 10,20,40,80,160,250,250,250", listOf(o.pauses).c_str());
   r.check(o.rc == 1 && o.critical == 1, "the loop ended before the script did (rc %d, %d critical)", o.rc, o.critical);
   r.fact("pauses %s", listOf(o.pauses).c_str());
   return r.result();
}

// T8: a persistent failure is reported once, repeated at most every 5 s of the monotonic clock, and its end is reported
int checkLoggingRateLimit(bool clobberErrno, FILE* note) {
   Report r(note);
   const int FAILURES = 1000;
   Steps steps;
   steps.fail(EMFILE, FAILURES).real();
   const LoopOutcome o = runScripted(steps, 1, "none", EBADF, clobberErrno);
   r.check(o.setupOk, "the server could not be set up");
   r.check(o.calls == FAILURES + 2, "%ld accept() calls", o.calls);
   r.check(o.echoed == 1, "the connection after the failures was not handled");
   const long long maxWarnings = 1 + o.virtualElapsedMs / 5000;
   r.check(o.warnings >= 2, "%d warnings: the failing streak was reported only once", o.warnings);
   r.check(o.warnings <= maxWarnings, "%d warnings in %lld virtual ms, at most %lld allowed (one per 5 s)", o.warnings, o.virtualElapsedMs, maxWarnings);
   r.check(o.warnings < FAILURES / 10, "%d warnings for %d failures", o.warnings, FAILURES);
   r.check(o.firstWarning.find(errnoText(EMFILE)) != std::string::npos, "the first warning does not name %s: '%s'", errnoText(EMFILE).c_str(), o.firstWarning.c_str());
   r.check(o.infos == 1, "%d recovery messages, expected 1", o.infos);
   r.check(o.firstInfo.find(std::to_string(FAILURES)) != std::string::npos, "the recovery message does not say how many attempts failed: '%s'", o.firstInfo.c_str());
   r.check(o.rc == 1 && o.critical == 1, "the loop did not end with the one terminal diagnostic (rc %d, %d critical)", o.rc, o.critical);
   r.fact("%d failures over %lld virtual ms: %d warnings, %d recovery; first warning: %s", FAILURES, o.virtualElapsedMs, o.warnings, o.infos, o.firstWarning.c_str());
   return r.result();
}

// T9: accepted connections go through the existing handler and (when threaded) thread-pool path
int checkSuccessfulDispatch(const char* threading, FILE* note) {
   Report r(note);
   Steps steps;
   steps.real(4);
   const LoopOutcome o = runScripted(steps, 4, threading);
   r.check(o.setupOk, "the server could not be set up");
   r.check(o.echoed == 4, "%d of 4 connections were handled (echoed) with threading=%s", o.echoed, threading);
   r.check(o.pauses.empty() && o.warnings == 0 && o.infos == 0, "a clean run produced pauses or diagnostics");
   r.check(o.rc == 1 && o.critical == 1, "rc %d, %d critical", o.rc, o.critical);
   r.fact("threading=%s: %d of 4 connections echoed", threading, o.echoed);
   return r.result();
}

//******************************************************************************
// integration: real kernel

// shared between the thread that runs the server and the test
struct Live {
   std::atomic<bool> done{false};
   std::atomic<int> rc{-99};
   std::atomic<long> calls{0};
   std::atomic<long> eintr{0};
   std::atomic<long> emfile{0};
   std::atomic<long> otherErrors{0};
   std::atomic<int> lastError{0};
   std::atomic<int> listenerFd{-1};
   std::atomic<int> inAccept{0};
   std::atomic<long> pauses{0};
   // The exhaustion test parks the accept thread at the entry of accept() (gate -> parked), fills the
   // descriptor table from the test thread while nothing else is using descriptors, then lets it go
   // (go): the next accept() call is made on a full table, nothing can free a slot first, and the
   // result is the kernel's.
   std::atomic<int> gate{0};
   std::atomic<int> parked{0};
   std::atomic<int> go{0};
   int fillErrno = 0;
   int limit = 48;
   int slotsOccupied = 0;
   std::vector<int> fill;
   std::atomic<pthread_t*> serverThread{nullptr};
   pthread_t threadId;
};

Live* g_live = nullptr;

// lowers this process's descriptor limit and takes every free slot below it: the next descriptor
// allocation fails with EMFILE. Returns true if the table is full (every slot below the limit is open).
bool fillDescriptorTable(Live& live) {
   struct rlimit rl;
   if (::getrlimit(RLIMIT_NOFILE, &rl) != 0) {
      return false;
   }
   rl.rlim_cur = static_cast<rlim_t>(live.limit);
   if (::setrlimit(RLIMIT_NOFILE, &rl) != 0) {
      return false;
   }
   for (;;) {
      const int f = ::open("/dev/null", O_RDONLY);
      if (f < 0) {
         live.fillErrno = errno;
         break;
      }
      live.fill.push_back(f);
   }
   int occupied = 0;
   for (int i = 0; i < live.limit; ++i) {
      if (::fcntl(i, F_GETFD) != -1) {
         ++occupied;
      }
   }
   live.slotsOccupied = occupied;
   return live.fillErrno == EMFILE && occupied == live.limit;
}

int liveAccept(int fd, sockaddr* address, socklen_t* addressLength) {
   Live* live = g_live;
   // inAccept is set before the gate is looked at, and the test thread sets the gate before it looks
   // at inAccept: at least one of them sees the other
   live->inAccept.store(1);
   if (live->gate.load() != 0) {
      live->inAccept.store(0);
      live->parked.store(1);
      const long long limit = nowMs() + 10000;
      while (live->go.load() == 0 && nowMs() < limit) {
         sleepMs(1);
      }
      live->inAccept.store(1);
   }
   if (live->listenerFd.load() < 0) {
      live->listenerFd.store(fd);
   }
   live->calls.fetch_add(1);
   const int result = ::accept(fd, address, addressLength);
   const int error = errno;
   live->inAccept.store(0);
   if (result < 0) {
      live->lastError.store(error);
      if (error == EINTR) {
         live->eintr.fetch_add(1);
      } else if (error == EMFILE) {
         live->emfile.fetch_add(1);
      } else {
         live->otherErrors.fetch_add(1);
      }
   }
   errno = error;
   return result;
}

void liveSleep(int millis) {
   g_live->pauses.fetch_add(1);
   sleepMs(millis);
}

std::chrono::steady_clock::time_point liveNow() {
   return std::chrono::steady_clock::now();
}

const detail::AcceptIoOps LIVE_ACCEPT_OPS = { liveAccept };
const detail::AcceptLoopOps LIVE_LOOP_OPS = { liveNow, liveSleep };

// the real server (default hooks plus the recording ones above) in its own thread
class LiveServer {
public:
   LiveServer(Live& live, int port, const char* threading) :
      m_live(live), m_config(writeConfig(port, threading)), m_server(nullptr), m_ok(false) {
      g_live = &live;
      if (!m_config.empty()) {
         m_server = new AcceptTestServer(m_config);
         detail::setAcceptIoOps(&LIVE_ACCEPT_OPS);
         detail::setAcceptLoopOps(&LIVE_LOOP_OPS);
         m_thread = std::thread([this]() {
            m_live.threadId = ::pthread_self();
            m_live.serverThread.store(&m_live.threadId);
            m_live.rc.store(m_server->run());
            m_live.done.store(true);
         });
         m_ok = true;
      }
   }
   ~LiveServer() {
      // the loop of a healthy server never returns: the thread is left to the end of the child
      if (m_thread.joinable()) {
         if (m_live.done.load()) {
            m_thread.join();
         } else {
            m_thread.detach();
         }
      }
      if (!m_config.empty()) {
         ::unlink(m_config.c_str());
      }
   }
   bool ok() const { return m_ok; }
private:
   Live& m_live;
   std::string m_config;
   AcceptTestServer* m_server;
   std::thread m_thread;
   bool m_ok;
};

bool waitFor(const std::function<bool()>& condition, long long millis) {
   const long long limit = nowMs() + millis;
   while (nowMs() < limit) {
      if (condition()) {
         return true;
      }
      sleepMs(2);
   }
   return condition();
}

// A: a closed listening descriptor ends the loop at once instead of spinning (real kernel EBADF)
int checkClosedListener(FILE* note) {
   Report r(note);
   LoggerScope logger;
   Live live;
   const int port = freePort();
   LiveServer server(live, port, "none");
   if (port < 0 || !server.ok()) {
      ::fprintf(note, "the server could not be set up");
      return CHILD_SETUP_FAILED;
   }

   // the loop must be blocked inside accept() on the listener
   if (!waitFor([&]() { return live.inAccept.load() == 1 && live.listenerFd.load() >= 0; }, 3000)) {
      ::fprintf(note, "the server never blocked in accept()");
      return CHILD_INCONCLUSIVE;
   }
   sleepMs(200);

   const int listener = live.listenerFd.load();
   const double cpuBefore = processCpuSeconds();
   // A descriptor number is reused at once: the socket that will carry the "kick" is created first, so that
   // it does not take the number of the listener. An accept() that is already blocked keeps its reference to
   // the listener after close(), so it still takes the first connection that arrives; the call after it meets
   // the closed descriptor.
   const int kick = ::socket(AF_INET, SOCK_STREAM, 0);
   ::close(listener);
   const long callsAtClose = live.calls.load();
   const long long closedAt = nowMs();

   sockaddr_in addr;
   ::memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   addr.sin_port = htons(static_cast<unsigned short>(port));
   if (::connect(kick, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0) {
      ::send(kick, "ping", 4, 0);
   }

   const bool ended = waitFor([&]() { return live.done.load(); }, 5000);
   const long long endedAfter = nowMs() - closedAt;
   const double cpu = processCpuSeconds() - cpuBefore;
   const long callsAfter = live.calls.load() - callsAtClose;
   const int lastError = live.lastError.load();
   if (kick >= 0) {
      ::close(kick);
   }

   r.check(ended, "run() did not return within 5 s of the listener being closed (accept() calls since: %ld)", callsAfter);
   r.check(live.rc.load() != 0 && live.rc.load() != -99, "run() returned %d", live.rc.load());
   r.check(callsAfter <= 6, "%ld accept() calls after the listener was closed, expected a handful", callsAfter);
   r.check(lastError == EBADF || lastError == EINVAL || lastError == ENOTSOCK, "the last accept() error was %s, expected EBADF, EINVAL or ENOTSOCK", errnoText(lastError).c_str());
   r.check(cpu < 1.0, "%.2f s of CPU after the listener was closed", cpu);
   r.check(logger->count(Critical) == 1, "%d critical diagnostics, expected 1", logger->count(Critical));
   r.check(logger->first(Critical).find(errnoText(lastError)) != std::string::npos, "the critical diagnostic does not name %s: '%s'", errnoText(lastError).c_str(), logger->first(Critical).c_str());
   r.fact("closed listener: run() returned %d %lld ms later; %ld accept() calls, last error %s (real kernel); %.3f s CPU; %d critical diagnostic: %s",
          live.rc.load(), endedAfter, callsAfter, errnoText(lastError).c_str(), cpu, logger->count(Critical), logger->first(Critical).c_str());
   return r.result();
}

// the client process of the exhaustion test: waits for a byte on goFd, then connects, sends "ping" and waits for the echo.
// Exit codes: 0 echo received, 10 no go signal, 11 connect failed, 12 no echo, 13 connection lost
[[noreturn]] void helperClient(int goFd, int port) {
   char go;
   if (::read(goFd, &go, 1) != 1) {
      ::_exit(10);
   }
   const int fd = connectClient(port);
   if (fd < 0) {
      ::_exit(11);
   }
   if (::send(fd, "ping", 4, 0) != 4) {
      ::_exit(13);
   }
   char buffer[4];
   size_t got = 0;
   const long long limit = nowMs() + 10000;
   while (got < 4) {
      if (!readableWithin(fd, limit - nowMs())) {
         ::_exit(12);
      }
      const ssize_t n = ::recv(fd, buffer + got, 4 - got, 0);
      if (n <= 0) {
         ::_exit(13);
      }
      got += static_cast<size_t>(n);
   }
   ::_exit(::memcmp(buffer, "ping", 4) == 0 ? 0 : 12);
}

// A second client process for the time the table is full: after the go signal it makes four connections, 100 ms
// apart, each closed at once without data. Where a failed accept() with EMFILE discards the connection it was
// working on (macOS), the accept loop blocks in accept() between connections; this gives every attempt a
// connection to fail on, so that the test sees real EMFILE results there too. Elsewhere the connections simply
// wait in the queue.
[[noreturn]] void helperFeeder(int goFd, int port) {
   char go;
   if (::read(goFd, &go, 1) != 1) {
      ::_exit(10);
   }
   for (int i = 0; i < 4; ++i) {
      const int fd = connectClient(port);
      if (fd < 0) {
         ::_exit(11);
      }
      sleepMs(100);
      ::close(fd);
   }
   ::_exit(0);
}

struct Helper {
   pid_t pid = -1;
   int goWrite = -1;
};

typedef void (*HelperBody)(int goFd, int port);

bool startHelper(Helper& h, int port, HelperBody body = helperClient) {
   int p[2];
   if (::pipe(p) != 0) {
      return false;
   }
   const pid_t pid = ::fork();
   if (pid < 0) {
      ::close(p[0]);
      ::close(p[1]);
      return false;
   }
   if (pid == 0) {
      ::close(p[1]);
      body(p[0], port);
      ::_exit(14);
   }
   ::close(p[0]);
   h.pid = pid;
   h.goWrite = p[1];
   return true;
}

// the exit code of a helper (waits at most millis), -1 if it is still running
int helperExit(Helper& h, long long millis) {
   const long long limit = nowMs() + millis;
   for (;;) {
      int status = 0;
      const pid_t w = ::waitpid(h.pid, &status, WNOHANG);
      if (w == h.pid) {
         h.pid = -1;
         return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
      }
      if (nowMs() >= limit) {
         return -1;
      }
      sleepMs(5);
   }
}

void stopHelper(Helper& h) {
   if (h.goWrite >= 0) {
      ::close(h.goWrite);
      h.goWrite = -1;
   }
   if (h.pid > 0) {
      ::kill(h.pid, SIGKILL);
      int status = 0;
      ::waitpid(h.pid, &status, 0);
      h.pid = -1;
   }
}

// B: real descriptor exhaustion. Real accept() returns EMFILE; the loop backs off instead of spinning;
// when descriptors are available again a fresh client (in a separate process, with its own descriptor
// table) is served. On Linux a connection that was queued while the table was full is served as well;
// macOS drops it (T2d-1a), which is not a defect of the server and is only recorded there.
int checkDescriptorExhaustionRecovery(FILE* note) {
   Report r(note);
   const int port = freePort();
   if (port < 0) {
      ::fprintf(note, "no free port");
      return CHILD_SETUP_FAILED;
   }
   Helper queued, fresh, feeder;   // forked now, before the server exists and before any limit is lowered
   if (!startHelper(queued, port) || !startHelper(fresh, port) || !startHelper(feeder, port, helperFeeder)) {
      stopHelper(queued);
      stopHelper(fresh);
      stopHelper(feeder);
      ::fprintf(note, "could not start the client processes");
      return CHILD_SETUP_FAILED;
   }

   LoggerScope logger;
   Live live;
   int result = CHILD_OK;
   {
      LiveServer server(live, port, "none");
      if (!server.ok()) {
         stopHelper(queued);
         stopHelper(fresh);
         stopHelper(feeder);
         ::fprintf(note, "the server could not be set up");
         return CHILD_SETUP_FAILED;
      }

      r.check(pingOnce(port, 5000), "the server did not serve a connection before the exhaustion");

      // park the accept thread at the entry of its next accept() call. If it is blocked inside accept() it
      // does not reach the entry until a connection arrives: that connection is served, then it parks.
      live.gate.store(1);
      if (!waitFor([&]() { return live.parked.load() == 1; }, 50)) {
         r.check(pingOnce(port, 5000), "the connection that moves the accept thread to the gate was not served");
      }
      if (!waitFor([&]() { return live.parked.load() == 1; }, 5000)) {
         stopHelper(queued);
         stopHelper(fresh);
         stopHelper(feeder);
         ::fprintf(note, "the accept thread did not reach the gate");
         return CHILD_INCONCLUSIVE;
      }

      // nothing else uses descriptors now: fill the table, then release the accept thread
      const bool full = fillDescriptorTable(live);
      if (!full) {
         live.go.store(1);
         stopHelper(queued);
         stopHelper(fresh);
         stopHelper(feeder);
         ::fprintf(note, "descriptor exhaustion was not established (open() errno %s, %d of %d slots occupied)", errnoText(live.fillErrno).c_str(), live.slotsOccupied, live.limit);
         return CHILD_INCONCLUSIVE;
      }
      live.go.store(1);

      // a client connects while the table is full
      ::write(queued.goWrite, "g", 1);
      ::write(feeder.goWrite, "g", 1);
      if (!waitFor([&]() { return live.emfile.load() >= 3; }, 5000)) {
         stopHelper(queued);
         stopHelper(fresh);
         stopHelper(feeder);
         ::fprintf(note, "accept() did not return EMFILE three times (%ld) with the table full; %ld accept() calls, %ld other errors, last error %s, in accept %d, %ld pauses, run() returned %d",
                   live.emfile.load(), live.calls.load(), live.otherErrors.load(), errnoText(live.lastError.load()).c_str(), live.inAccept.load(), live.pauses.load(), live.done.load() ? live.rc.load() : -99);
         return CHILD_INCONCLUSIVE;
      }

      // the loop must be backing off: a bounded number of accept() calls and little CPU over a second
      const long callsBefore = live.calls.load();
      const double cpuBefore = processCpuSeconds();
      sleepMs(1000);
      const long callsInWindow = live.calls.load() - callsBefore;
      const double cpuInWindow = processCpuSeconds() - cpuBefore;
      r.check(callsInWindow <= 30, "%ld accept() calls in one second with the table full (10..250 ms pauses allow about 5)", callsInWindow);
      r.check(cpuInWindow < 0.3, "%.2f s of CPU in one second with the table full", cpuInWindow);
      r.check(!live.done.load(), "run() returned while the table was full (rc %d)", live.rc.load());
      r.check(live.otherErrors.load() == 0, "accept() returned an error other than EMFILE (%s)", errnoText(live.lastError.load()).c_str());
      const long emfileSeen = live.emfile.load();
      const int warningsWhileFull = logger->count(Warning);
      r.check(warningsWhileFull >= 1 && warningsWhileFull <= 2, "%d warnings while the table was full, expected 1 (at most 2)", warningsWhileFull);
      r.check(logger->count(Critical) == 0, "a critical diagnostic was written while the table was full");

      // descriptors are available again: close what this process filled with
      for (int fd : live.fill) {
         ::close(fd);
      }
      live.fill.clear();
      const long long releasedAt = nowMs();

      // a fresh client, in another process, so that it cannot use the descriptors that were just freed
      ::write(fresh.goWrite, "g", 1);
      const int freshExit = helperExit(fresh, 15000);
      const long long freshAfter = nowMs() - releasedAt;
      const int queuedExit = helperExit(queued, 8000);

      r.check(freshExit == 0, "the fresh client was not served after the descriptors were released (client exit code %d)", freshExit);
      r.check(!live.done.load(), "run() returned (rc %d)", live.rc.load());
      r.check(logger->count(Info) == 1, "%d recovery messages, expected 1", logger->count(Info));
      r.check(logger->count(Critical) == 0, "a critical diagnostic was written");
#if defined(__linux__)
      // observed on Linux (T2d-1): a failed accept() with EMFILE leaves the queued connection in the queue
      r.check(queuedExit == 0, "the connection that was queued while the table was full was not served (client exit code %d)", queuedExit);
#endif
      r.fact("real EMFILE: %ld accept() calls returned it, %d of %d slots were occupied; %ld calls and %.3f s CPU in the 1 s window; %ld pauses; "
             "fresh client served %lld ms after release (exit %d); client queued while full: exit %d (%s); %d warning, %d recovery",
             emfileSeen, live.slotsOccupied, live.limit, callsInWindow, cpuInWindow, live.pauses.load(), freshAfter, freshExit, queuedExit,
#if defined(__linux__)
             "required on Linux",
#else
             "informational: macOS drops it",
#endif
             warningsWhileFull, logger->count(Info));
      result = r.result();
   }
   stopHelper(queued);
   stopHelper(fresh);
   stopHelper(feeder);
   return result;
}

std::atomic<int> g_usr1Count(0);
std::atomic<long> g_usr1OnServer(0);

void onUsr1(int) {
   g_usr1Count.fetch_add(1);
   Live* live = g_live;
   if (live != nullptr) {
      pthread_t* server = live->serverThread.load();
      if (server != nullptr && ::pthread_equal(::pthread_self(), *server)) {
         g_usr1OnServer.fetch_add(1);
      }
   }
}

// C: a real signal that makes the blocked accept() return EINTR does not stop the server (SIGUSR1, no SA_RESTART)
int checkRealSignalEintr(FILE* note) {
   Report r(note);
   LoggerScope logger;
   Live live;
   const int port = freePort();

   struct sigaction action;
   ::memset(&action, 0, sizeof(action));
   action.sa_handler = onUsr1;
   sigemptyset(&action.sa_mask);
   action.sa_flags = 0;                 // no SA_RESTART: the kernel returns EINTR
   ::sigaction(SIGUSR1, &action, nullptr);

   LiveServer server(live, port, "none");
   if (port < 0 || !server.ok()) {
      ::fprintf(note, "the server could not be set up");
      return CHILD_SETUP_FAILED;
   }
   if (!waitFor([&]() { return live.inAccept.load() == 1 && live.serverThread.load() != nullptr; }, 3000)) {
      ::fprintf(note, "the server never blocked in accept()");
      return CHILD_INCONCLUSIVE;
   }
   sleepMs(250);
   if (live.inAccept.load() != 1) {
      ::fprintf(note, "accept() is not blocked 250 ms after the server started");
      return CHILD_INCONCLUSIVE;
   }

   for (int i = 0; i < 5; ++i) {
      const int before = g_usr1Count.load();
      ::pthread_kill(*live.serverThread.load(), SIGUSR1);
      const long long limit = nowMs() + 300;
      while (g_usr1Count.load() == before && nowMs() < limit) {
         sleepMs(2);
      }
      sleepMs(60);
   }

   const long eintr = live.eintr.load();
   char facts[300];
   ::snprintf(facts, sizeof(facts), "%d signals handled, %ld on the server thread, %ld accept() calls, %ld returned EINTR (real kernel), %ld pauses",
              g_usr1Count.load(), g_usr1OnServer.load(), live.calls.load(), eintr, live.pauses.load());
   if (g_usr1OnServer.load() < 1) {
      ::fprintf(note, "no signal handler ran on the thread blocked in accept(); %s", facts);
      return CHILD_INCONCLUSIVE;
   }
   if (eintr < 1) {
      ::fprintf(note, "no accept() call returned EINTR, so the retry was not exercised; %s", facts);
      return CHILD_INCONCLUSIVE;
   }

   r.check(pingOnce(port, 5000), "the server did not serve a connection after the interruptions");
   r.check(!live.done.load(), "run() returned (rc %d)", live.rc.load());
   r.check(live.pauses.load() == 0, "the loop paused %ld times for interruptions", live.pauses.load());
   r.check(logger->count(Warning) == 0 && logger->count(Critical) == 0, "diagnostics were written for interruptions (%d warnings, %d critical)", logger->count(Warning), logger->count(Critical));
   r.check(live.otherErrors.load() == 0, "accept() returned an error other than EINTR (%s)", errnoText(live.lastError.load()).c_str());
   r.fact("%s; the server then served a connection", facts);
   return r.result();
}

}

//******************************************************************************

#define JUDGE_CHILD(WHAT, RESULT)                                                          \
   do {                                                                                    \
      if ((RESULT).exited && ((RESULT).exitCode == CHILD_INCONCLUSIVE)) {                  \
         printf("INCONCLUSIVE: %s: %s\n", (WHAT), (RESULT).diagnostics.c_str());           \
         fflush(stdout);                                                                   \
      } else {                                                                             \
         require((RESULT).exited && ((RESULT).exitCode == CHILD_OK),                       \
                 describe((WHAT), (RESULT)));                                              \
         if ((RESULT).exited && ((RESULT).exitCode == CHILD_OK)) {                         \
            printf("OBSERVED: %s: %s\n", (WHAT), (RESULT).diagnostics.c_str());            \
            fflush(stdout);                                                                \
         }                                                                                 \
      }                                                                                    \
   } while (0)

//******************************************************************************
// policy tests: the classifier and the tracker the server uses, no sockets, no clock

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testClassifierClasses) {
   using detail::AcceptErrorClass;
   using detail::classifyAcceptError;

   // Class 1
   const int immediate[] = {
      EINTR, ECONNABORTED,
#ifdef EPROTO
      EPROTO,
#endif
#ifdef ENETDOWN
      ENETDOWN,
#endif
#ifdef ENOPROTOOPT
      ENOPROTOOPT,
#endif
#ifdef EHOSTDOWN
      EHOSTDOWN,
#endif
#ifdef ENONET
      ENONET,
#endif
#ifdef EHOSTUNREACH
      EHOSTUNREACH,
#endif
#ifdef ENETUNREACH
      ENETUNREACH,
#endif
   };
   for (int e : immediate) {
      require(classifyAcceptError(e) == AcceptErrorClass::RetryImmediately, "errno " + std::to_string(e) + " is retried immediately");
   }

   // Class 2 (the resource errors), Class 4 for what is not listed
   const int backOff[] = {
      EMFILE, ENFILE, ENOBUFS, ENOMEM, EAGAIN, EWOULDBLOCK, EOPNOTSUPP,
      EIO, EPERM, ENXIO, ETIMEDOUT, EACCES, EEXIST, EPIPE, ECONNRESET, 0, 9999, -1
   };
   for (int e : backOff) {
      require(classifyAcceptError(e) == AcceptErrorClass::BackOff, "errno " + std::to_string(e) + " backs off");
   }

   // Class 3
   const int terminal[] = { EBADF, ENOTSOCK, EINVAL, EFAULT };
   for (int e : terminal) {
      require(classifyAcceptError(e) == AcceptErrorClass::Terminal, "errno " + std::to_string(e) + " is terminal");
   }

   require(classifyAcceptError(EAGAIN) == classifyAcceptError(EWOULDBLOCK), "EAGAIN and EWOULDBLOCK are classified alike");
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testTrackerDecisions) {
   using namespace detail;
   typedef AcceptFailureTracker Tracker;
   const Tracker::TimePoint t0 = std::chrono::steady_clock::time_point(std::chrono::seconds(100));

   // the pause sequence, whatever the errors are
   {
      Tracker t;
      const int errors[] = { EMFILE, ENOMEM, EMFILE, EAGAIN, ENFILE, ENOBUFS, EIO, EMFILE };
      const int expected[] = { 10, 20, 40, 80, 160, 250, 250, 250 };
      for (size_t i = 0; i < 8; ++i) {
         const AcceptDecision d = t.onFailure(errors[i], t0);
         require(d.action == AcceptAction::PauseThenRetry, "a resource error is retried after a pause");
         require(d.pauseMillis == expected[i], "pause " + std::to_string(i + 1) + " is " + std::to_string(expected[i]) + " ms, got " + std::to_string(d.pauseMillis));
      }
   }

   // the immediate-retry guard: exactly 100 immediate retries, then pauses
   {
      Tracker t;
      for (int i = 1; i <= 100; ++i) {
         const AcceptDecision d = t.onFailure(EINTR, t0);
         require(d.action == AcceptAction::RetryNow, "immediate retry " + std::to_string(i) + " of 100");
      }
      const AcceptDecision d101 = t.onFailure(EINTR, t0);
      require(d101.action == AcceptAction::PauseThenRetry && d101.pauseMillis == 10, "the 101st failure in a row is paused");
      const AcceptDecision d102 = t.onFailure(ECONNABORTED, t0);
      require(d102.action == AcceptAction::PauseThenRetry && d102.pauseMillis == 20, "so is the 102nd, with a longer pause, whatever the immediate-retry error");
   }

   // a success resets the immediate count, the pause and the streak
   {
      Tracker t;
      for (int i = 0; i < 100; ++i) {
         t.onFailure(EINTR, t0);
      }
      t.onFailure(EMFILE, t0);
      t.onFailure(EMFILE, t0);
      const AcceptRecovery recovery = t.onSuccess();
      require(recovery.wasReported && recovery.failuresInStreak == 2, "the recovery reports the streak that was logged");
      for (int i = 1; i <= 100; ++i) {
         require(t.onFailure(EINTR, t0).action == AcceptAction::RetryNow, "after a success the immediate retries start again");
      }
      require(t.onFailure(EMFILE, t0).pauseMillis == 10, "after a success the pause starts again at 10 ms");
   }

   // a change of errno does not reset the pause, and an immediate-retry error is not a reset
   {
      Tracker t;
      require(t.onFailure(EMFILE, t0).pauseMillis == 10, "10");
      require(t.onFailure(EINTR, t0).action == AcceptAction::RetryNow, "EINTR between two resource errors is retried at once");
      require(t.onFailure(ENOMEM, t0).pauseMillis == 20, "20 after a different errno");
      require(t.onFailure(EINTR, t0).action == AcceptAction::RetryNow, "again");
      require(t.onFailure(EAGAIN, t0).pauseMillis == 40, "40 after a third errno");
   }

   // terminal errors stop, whatever happened before
   {
      Tracker t;
      t.onFailure(EMFILE, t0);
      const int terminal[] = { EBADF, ENOTSOCK, EINVAL, EFAULT };
      for (int e : terminal) {
         const AcceptDecision d = t.onFailure(e, t0);
         require(d.action == AcceptAction::Stop && d.log == AcceptLog::Terminal, "a terminal error stops and is reported");
         require(d.errorClass == AcceptErrorClass::Terminal, "classified as terminal");
      }
   }

   // the pause never exceeds its maximum however long the streak is
   {
      Tracker t;
      int last = 0;
      for (int i = 0; i < 100000; ++i) {
         last = t.onFailure(EMFILE, t0).pauseMillis;
      }
      require(last == MAX_BACKOFF_MILLIS, "the pause stays at its maximum");
   }
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testTrackerLoggingRateLimit) {
   using namespace detail;
   typedef AcceptFailureTracker Tracker;
   const Tracker::TimePoint t0 = std::chrono::steady_clock::time_point(std::chrono::seconds(100));

   Tracker t;
   // failures that are retried at once are not reported
   for (int i = 0; i < 100; ++i) {
      require(t.onFailure(EINTR, t0).log == AcceptLog::Nothing, "an immediate retry is not reported");
   }
   require(!t.onSuccess().wasReported, "a streak that was never reported has no recovery message");

   // the first failure that is paused is reported, the next ones are not, until 5 s have passed
   require(t.onFailure(EMFILE, t0).log == AcceptLog::StreakStarted, "the start of the streak is reported");
   for (int i = 1; i <= 50; ++i) {
      const AcceptDecision d = t.onFailure(EMFILE, t0 + std::chrono::milliseconds(i * 90));    // up to 4.5 s
      require(d.log == AcceptLog::Nothing, "a repeat inside the interval is not reported");
   }
   const AcceptDecision repeat = t.onFailure(EMFILE, t0 + std::chrono::milliseconds(5000));
   require(repeat.log == AcceptLog::StreakContinues, "a repeat after 5 s is reported");
   require(repeat.suppressed == 50, "it says how many were not reported (" + std::to_string(repeat.suppressed) + ")");
   require(t.onFailure(EMFILE, t0 + std::chrono::milliseconds(5001)).log == AcceptLog::Nothing, "and the interval starts again");
   require(t.onFailure(EMFILE, t0 + std::chrono::milliseconds(9999)).log == AcceptLog::Nothing, "just before the next 5 s");
   require(t.onFailure(EMFILE, t0 + std::chrono::milliseconds(10000)).log == AcceptLog::StreakContinues, "at 10 s");

   // a success ends the streak (reported once) and the next streak starts with a fresh report
   const AcceptRecovery recovery = t.onSuccess();
   require(recovery.wasReported, "the end of a reported streak is reported");
   require(!t.onSuccess().wasReported, "and only once");
   require(t.onFailure(EMFILE, t0 + std::chrono::milliseconds(10001)).log == AcceptLog::StreakStarted, "the next streak is reported at once, inside the old interval");
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testAcceptHooksAreRestored) {
   require(&detail::acceptIoOps() == &detail::systemAcceptIoOps(), "ServerSocket::accept() uses the real accept() by default");
   require(&detail::acceptLoopOps() == &detail::systemAcceptLoopOps(), "the accept loop uses the real clock and pause by default");
   {
      const detail::AcceptIoOps* previous = detail::setAcceptIoOps(&SCRIPTED_ACCEPT_OPS);
      require(previous == &detail::systemAcceptIoOps(), "the previous ops are returned");
      require(&detail::acceptIoOps() == &SCRIPTED_ACCEPT_OPS, "an override takes effect");
      detail::setAcceptIoOps(previous);
   }
   {
      const detail::AcceptLoopOps* previous = detail::setAcceptLoopOps(&VIRTUAL_LOOP_OPS);
      require(&detail::acceptLoopOps() == &VIRTUAL_LOOP_OPS, "an override of the loop ops takes effect");
      detail::setAcceptLoopOps(previous);
   }
   require(&detail::acceptIoOps() == &detail::systemAcceptIoOps(), "the real accept() is back");
   require(&detail::acceptLoopOps() == &detail::systemAcceptLoopOps(), "the real clock and pause are back");
   require(detail::setAcceptIoOps(nullptr) == &detail::systemAcceptIoOps(), "nullptr selects the real accept()");
   require(detail::setAcceptLoopOps(nullptr) == &detail::systemAcceptLoopOps(), "nullptr selects the real loop ops");
}

//******************************************************************************
// scripted loop tests (injected results, real loop, real handlers)

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testImmediateRetryThenSuccess) {
   const ChildResult r = runInChild(checkImmediateRetry, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T1 EINTR, EINTR, success (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testPersistentImmediateRetryErrorIsBounded) {
   const ChildResult r = runInChild(checkImmediateRetryGuard, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T2 250 EINTR in a row (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testExponentialBackoff) {
   const ChildResult r = runInChild(checkExponentialBackoff, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T3 persistent EMFILE (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testBackoffResetsAfterSuccess) {
   const ChildResult r = runInChild(checkBackoffResetsAfterSuccess, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T4 EMFILE, EMFILE, success, EMFILE (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testAlternatingErrorsDoNotResetBackoff) {
   const ChildResult r = runInChild(checkAlternatingErrors, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T5 alternating errors (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testTerminalErrorEBADF) {
   const ChildResult r = runInChild([](FILE* note) { return checkTerminalError(EBADF, note); }, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T6 EBADF (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testTerminalErrorENOTSOCK) {
   const ChildResult r = runInChild([](FILE* note) { return checkTerminalError(ENOTSOCK, note); }, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T6 ENOTSOCK (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testTerminalErrorEINVAL) {
   const ChildResult r = runInChild([](FILE* note) { return checkTerminalError(EINVAL, note); }, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T6 EINVAL (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testTerminalErrorEFAULT) {
   const ChildResult r = runInChild([](FILE* note) { return checkTerminalError(EFAULT, note); }, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T6 EFAULT (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testUnknownErrorsBackOff) {
   const ChildResult r = runInChild(checkUnknownErrors, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T7 unknown errors (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testLoggingIsRateLimited) {
   const ChildResult r = runInChild([](FILE* note) { return checkLoggingRateLimit(false, note); }, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T8 logging rate limit (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testLoggingDoesNotAlterClassification) {
   // a logger that sets errno to EBADF (terminal) in every call must not change what the loop does with EMFILE
   const ChildResult r = runInChild([](FILE* note) { return checkLoggingRateLimit(true, note); }, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T8 logging with a logger that clobbers errno (injected)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testSuccessfulDispatchWithoutThreadPool) {
   const ChildResult r = runInChild([](FILE* note) { return checkSuccessfulDispatch("none", note); }, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T9 dispatch without a thread pool", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testSuccessfulDispatchThroughThreadPool) {
   const ChildResult r = runInChild([](FILE* note) { return checkSuccessfulDispatch("pthreads", note); }, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("T9 dispatch through the thread pool", r);
}

//******************************************************************************
// integration tests against the real kernel

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testClosedListenerEndsTheLoop) {
   const ChildResult r = runInChild(checkClosedListener, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("A closed listener (real accept error)", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testDescriptorExhaustionRecovery) {
   const ChildResult r = runInChild(checkDescriptorExhaustionRecovery, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("B real descriptor exhaustion and recovery", r);
}

POIVRE_TEST_CASE(TestSocketServerAcceptErrors, testRealSignalInterruptedAcceptIsRetried) {
   const ChildResult r = runInChild(checkRealSignalEintr, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("C real signal interrupts a blocked accept", r);
}
