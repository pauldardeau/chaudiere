// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

// Tests that a blocking connect() interrupted by a signal does not make Socket(address, port)
// give up a connection attempt that is still going on (defect F-01).
//
// connect() can fail with EINTR while the connection attempt goes on in the kernel (Linux and
// macOS both; macOS also when the handler was installed with SA_RESTART). Socket::open() used
// to treat that as the failure of the connection. It now waits for the attempt to end on the
// same descriptor with poll(POLLOUT) and reads the outcome with getsockopt(SO_ERROR); see
// chaudiere::detail::awaitConnectCompletion() in src/internal/SocketIo.h.
//
// Real signals: the integration tests make connect() block (a loopback listener whose accept
// queue is full, so that its SYN is dropped and retransmitted), then deliver SIGUSR1, with a
// handler installed without SA_RESTART, to the one thread that is blocked in connect(). A
// signal that was delivered is not taken as proof that connect() was interrupted: the
// wait for the connection is entered only after connect() returned EINTR, so the test replaces
// poll() with a counting one (chaudiere::detail::setConnectIoOps()) and a connection that
// completed without that poll() ever being called is reported INCONCLUSIVE, not as a pass.
//
// Injected poll(): the kernel cannot be made to return a second EINTR, an unrelated error, or a
// meaningless result from poll() on demand, so those cases run the same real interrupted
// connect() but with a scripted poll() behind it.
//
// Results: a check ends PASS, FAIL, or INCONCLUSIVE when the environment cannot establish the
// condition under test (the listener cannot be saturated, connect() did not block, the signal
// did not run in the connecting thread, connect() was not interrupted). INCONCLUSIVE is printed
// as such and is never counted as evidence that the recovery works.
//
// Isolation and limits: every integration check runs in a forked child (ChildProcess.h) with a
// hard time limit, so descriptors it uses, the signal handler it installs and a regression that
// blocks cannot disturb the run. Loopback only, ports chosen by the kernel, no root.

#include <atomic>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "TestSocketConnectInterrupt.h"
#include "ChildProcess.h"
#include "BasicException.h"
#include "Socket.h"
#include "internal/SocketIo.h"

using namespace chaudiere;
using namespace chaudiere_test;

//******************************************************************************

TestSocketConnectInterrupt::TestSocketConnectInterrupt() :
   poivre::TestSuite("TestSocketConnectInterrupt") {
}

//******************************************************************************

namespace {

const int CHILD_INCONCLUSIVE = CHILD_SKIPPED;
const int TIME_LIMIT_MILLIS = 40000;
const long long SAFETY_MILLIS = 15000;      // the check's own limit for one connection to end
const long long BLOCK_CHECK_MILLIS = 250;   // connect() must still be waiting this long after it started
const long long RELIEVE_AT_MILLIS = 1300;   // when the full listener is relieved (or closed)

long long nowMs() {
   return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
}

void sleepMs(long long ms) {
   std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// POSIX gives a new descriptor the lowest number that is free (see TestSocketDescriptors.cpp)
int lowestFreeDescriptor() {
   const int fd = ::open("/dev/null", O_RDONLY);
   if (fd >= 0) {
      ::close(fd);
   }
   return fd;
}

sockaddr_in loopbackAddress(int port) {
   sockaddr_in addr;
   ::memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   addr.sin_port = htons(static_cast<unsigned short>(port));
   return addr;
}

void setNonBlocking(int fd, bool on) {
   const int flags = ::fcntl(fd, F_GETFL, 0);
   if (flags >= 0) {
      ::fcntl(fd, F_SETFL, on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK));
   }
}

int socketErrorOf(int fd) {
   int error = -1;
   socklen_t length = sizeof(error);
   if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length) != 0) {
      return -errno;
   }
   return error;
}

int portOf(int fd, bool peer) {
   sockaddr_in addr;
   socklen_t length = sizeof(addr);
   const int rc = peer ? ::getpeername(fd, reinterpret_cast<sockaddr*>(&addr), &length)
                       : ::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &length);
   return (rc == 0) ? ntohs(addr.sin_port) : -1;
}

// a non-blocking loopback listener on a kernel-chosen port
int listenLoopback(int backlog, int& port) {
   const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0) {
      return -1;
   }
   const int on = 1;
   ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
   sockaddr_in addr = loopbackAddress(0);
   if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 ||
       ::listen(fd, backlog) != 0) {
      ::close(fd);
      return -1;
   }
   port = portOf(fd, false);
   setNonBlocking(fd, true);
   return fd;
}

// a loopback port on which nothing listens
int unusedPort() {
   int port = -1;
   const int fd = listenLoopback(1, port);
   if (fd >= 0) {
      ::close(fd);
   }
   return port;
}

// takes every connection that is waiting; accepted descriptors are made blocking (BSD-derived
// systems pass the listener's O_NONBLOCK on to them)
void drainAccept(int listenFd, std::vector<int>& accepted) {
   for (;;) {
      const int fd = ::accept(listenFd, nullptr, nullptr);
      if (fd < 0) {
         return;
      }
      setNonBlocking(fd, false);
      accepted.push_back(fd);
   }
}

// Fills the accept queue with non-blocking connects until further ones stay pending (their SYN
// is dropped). True if saturation was established: two in a row stayed pending.
bool saturate(int port, std::vector<int>& fillers, FILE* note) {
   sockaddr_in addr = loopbackAddress(port);
   int consecutivePending = 0;
   int absorbed = 0;
   int pending = 0;
   int refused = 0;

   while (fillers.size() < 64 && consecutivePending < 2) {
      const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
      if (fd < 0) {
         break;
      }
      setNonBlocking(fd, true);
      if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 && errno != EINPROGRESS) {
         ::close(fd);
         break;
      }
      fillers.push_back(fd);

      pollfd p;
      p.fd = fd;
      p.events = POLLOUT;
      p.revents = 0;
      const int n = ::poll(&p, 1, 150);
      if (n > 0 && socketErrorOf(fd) == 0) {
         ++absorbed;
         consecutivePending = 0;
      } else if (n > 0) {
         ++refused;      // this stack answers an overflowing connect with an error instead of dropping it
         break;
      } else {
         ++pending;
         ++consecutivePending;
      }
   }

   fprintf(note, "saturation: %zu fillers, %d absorbed, %d pending, %d refused; ",
           fillers.size(), absorbed, pending, refused);
   return consecutivePending >= 2;
}

bool readable(int fd, long long millis) {
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

// the server side of the usability check: read PING, answer PONG
bool answerPing(int fd) {
   char b[4];
   if (!readable(fd, 3000)) {
      return false;
   }
   if (::recv(fd, b, sizeof(b), 0) != 4 || ::memcmp(b, "PING", 4) != 0) {
      return false;
   }
   return ::send(fd, "PONG", 4, 0) == 4;
}

// the client side: PING out, PONG back, through the Chaudiere Socket
bool pingPong(Socket& socket) {
   const char ping[4] = { 'P', 'I', 'N', 'G' };
   if (!socket.write(ping, 4)) {
      return false;
   }
   char b[4];
   ::memset(b, 0, sizeof(b));
   return socket.readSocket(b, 4, 4000) == 4 && ::memcmp(b, "PONG", 4) == 0;
}

//******************************************************************************
// the signal: SIGUSR1, no SA_RESTART, records which thread ran the handler

// The handler runs in the connecting thread and the main thread reads what it left, so these are
// atomics (lock-free, hence safe to use in a handler), not volatile variables.
static_assert(std::atomic<int>::is_always_lock_free, "the handler counter must be lock-free");
static_assert(std::atomic<pthread_t>::is_always_lock_free, "the handler thread id must be lock-free");

std::atomic<int> g_handlerCount(0);
std::atomic<pthread_t> g_handlerThread;

void onSignal(int) {
   g_handlerThread.store(pthread_self());
   g_handlerCount.fetch_add(1);
}

//******************************************************************************
// the poll() behind the wait for an interrupted connection

std::atomic<int> g_pollCalls(0);          // calls of the replacement poll
std::atomic<int> g_pollInjected(0);       // results it made up
std::atomic<int> g_pollRealEintr(0);      // EINTR that the kernel returned from a real poll()
int g_injectEintr = 0;                    // how many EINTR eintrThenRealPoll makes up first
int g_failErrno = EIO;

int realPollCounting(struct pollfd* fds, nfds_t nfds, int timeoutMillis) {
   const int result = ::poll(fds, nfds, timeoutMillis);
   const int savedErrno = errno;
   if (result < 0 && savedErrno == EINTR) {
      ++g_pollRealEintr;
   }
   errno = savedErrno;
   return result;
}

int countingPoll(struct pollfd* fds, nfds_t nfds, int timeoutMillis) {
   ++g_pollCalls;
   return realPollCounting(fds, nfds, timeoutMillis);
}

int eintrThenRealPoll(struct pollfd* fds, nfds_t nfds, int timeoutMillis) {
   ++g_pollCalls;
   if (g_pollInjected < g_injectEintr) {
      ++g_pollInjected;
      errno = EINTR;
      return -1;
   }
   return realPollCounting(fds, nfds, timeoutMillis);
}

int failingPoll(struct pollfd*, nfds_t, int) {
   ++g_pollCalls;
   ++g_pollInjected;
   errno = g_failErrno;
   return -1;
}

int invalidDescriptorPoll(struct pollfd* fds, nfds_t, int) {
   ++g_pollCalls;
   ++g_pollInjected;
   fds[0].revents = POLLNVAL;
   return 1;
}

int silentPoll(struct pollfd* fds, nfds_t, int) {
   ++g_pollCalls;
   ++g_pollInjected;
   fds[0].revents = 0;
   return 0;
}

int writableOnlyPoll(struct pollfd* fds, nfds_t, int) {
   ++g_pollCalls;
   fds[0].revents = POLLOUT;
   return 1;
}

int hangupPoll(struct pollfd* fds, nfds_t, int) {
   ++g_pollCalls;
   fds[0].revents = POLLOUT | POLLHUP;
   return 1;
}

ssize_t realRecv(int fd, void* buffer, size_t length, int flags) {
   return ::recv(fd, buffer, length, flags);
}

const detail::SocketIoOps COUNTING_OPS = { countingPoll, realRecv };
const detail::SocketIoOps EINTR_THEN_REAL_OPS = { eintrThenRealPoll, realRecv };
const detail::SocketIoOps FAILING_OPS = { failingPoll, realRecv };
const detail::SocketIoOps INVALID_DESCRIPTOR_OPS = { invalidDescriptorPoll, realRecv };
const detail::SocketIoOps SILENT_OPS = { silentPoll, realRecv };
const detail::SocketIoOps WRITABLE_ONLY_OPS = { writableOnlyPoll, realRecv };
const detail::SocketIoOps HANGUP_OPS = { hangupPoll, realRecv };

void resetCounters() {
   g_pollCalls = 0;
   g_pollInjected = 0;
   g_pollRealEintr = 0;
   g_handlerCount = 0;
}

//******************************************************************************
// one real interrupted connect()

enum Ending {
   END_RELIEVE,            // the listener's queue is drained: the attempt can complete
   END_REFUSE,             // the listener is closed: the attempt is refused
   END_WAIT_FOR_RETURN     // nothing is done: the constructor is expected to return by itself
};

struct Outcome {
   std::atomic<bool> constructed;
   bool threw = false;
   bool notBasicException = false;
   bool exchanged = false;
   std::string what;
   int errnoAtCatch = 0;
   int polls = 0;
   int injected = 0;
   int realEintr = 0;
   int signalsHandled = 0;
   int fdBefore = -1;
   int fdAfter = -1;
   Outcome() : constructed(false) {}
};

// Builds the situation, delivers the signal, ends the situation as asked and reports what the
// Socket constructor did. Returns CHILD_OK when it ran (the caller judges the outcome),
// CHILD_INCONCLUSIVE (with the reason in note) when the condition could not be established.
int runInterruptedConnect(FILE* note, Ending ending, const detail::SocketIoOps* ops,
                          int extraSignals, Outcome& out) {
   resetCounters();
   out.fdBefore = lowestFreeDescriptor();

   int port = 0;
   int listenFd = listenLoopback(1, port);
   if (listenFd < 0) {
      fprintf(note, "could not create a listener");
      return CHILD_SETUP_FAILED;
   }

   std::vector<int> fillers;
   std::vector<int> accepted;
   auto closeAll = [&]() {
      for (int fd : fillers) { ::close(fd); }
      fillers.clear();
      for (int fd : accepted) { ::close(fd); }
      accepted.clear();
      if (listenFd >= 0) { ::close(listenFd); listenFd = -1; }
   };

   if (!saturate(port, fillers, note)) {
      closeAll();
      fprintf(note, "the listener's queue could not be saturated, so a connect() that blocks cannot be built here");
      return CHILD_INCONCLUSIVE;
   }

   struct sigaction action;
   struct sigaction previousAction;
   ::memset(&action, 0, sizeof(action));
   action.sa_handler = onSignal;
   ::sigemptyset(&action.sa_mask);
   action.sa_flags = 0;                   // deliberately no SA_RESTART
   ::sigaction(SIGUSR1, &action, &previousAction);

   // only the connecting thread may run the handler
   sigset_t usr1;
   sigset_t previousMask;
   ::sigemptyset(&usr1);
   ::sigaddset(&usr1, SIGUSR1);
   ::pthread_sigmask(SIG_BLOCK, &usr1, &previousMask);

   if (ops != nullptr) {
      detail::setConnectIoOps(ops);
   }

   std::atomic<bool> started(false);
   std::atomic<bool> finished(false);
   std::atomic<long long> startedAt(0);
   std::atomic<int> localPort(0);

   std::thread worker([&]() {
      sigset_t unblock;
      ::sigemptyset(&unblock);
      ::sigaddset(&unblock, SIGUSR1);
      ::pthread_sigmask(SIG_UNBLOCK, &unblock, nullptr);

      startedAt = nowMs();
      started = true;
      try {
         Socket socket("127.0.0.1", port);
         out.constructed = true;
         localPort = portOf(socket.getFileDescriptor(), false);
         out.exchanged = pingPong(socket);
      } catch (const BasicException& be) {
         out.errnoAtCatch = errno;
         out.threw = true;
         out.what = be.whatString();
      } catch (...) {
         out.errnoAtCatch = errno;
         out.threw = true;
         out.notBasicException = true;
      }
      finished = true;
   });

   bool served = false;
   auto serve = [&]() {
      drainAccept(listenFd, accepted);
      const int lp = localPort;
      if (!served && lp > 0) {
         for (int fd : accepted) {
            if (portOf(fd, true) == lp) {
               served = answerPing(fd);
               if (!served) {
                  served = true;       // looked at once; the client reports the failed exchange
               }
               break;
            }
         }
      }
   };
   auto relieve = [&]() {
      for (int fd : fillers) { ::close(fd); }
      fillers.clear();
      drainAccept(listenFd, accepted);
   };
   auto wrapUp = [&]() {
      worker.join();
      detail::setConnectIoOps(nullptr);
      ::sigaction(SIGUSR1, &previousAction, nullptr);
      ::pthread_sigmask(SIG_SETMASK, &previousMask, nullptr);
      closeAll();
      out.polls = g_pollCalls;
      out.injected = g_pollInjected;
      out.realEintr = g_pollRealEintr;
      out.signalsHandled = g_handlerCount;
      out.fdAfter = lowestFreeDescriptor();
   };
   // used when the situation cannot be established: let the attempt end, then clean up
   auto abandonInconclusive = [&](const char* reason) -> int {
      relieve();
      const long long limit = nowMs() + SAFETY_MILLIS;
      while (!finished && nowMs() < limit) {
         drainAccept(listenFd, accepted);
         sleepMs(10);
      }
      if (!finished) {
         fflush(note);
         ::_exit(CHILD_CHECK_FAILED);       // cannot join a worker that is stuck; the child ends here
      }
      wrapUp();
      fprintf(note, "%s", reason);
      return CHILD_INCONCLUSIVE;
   };

   while (!started) {
      sleepMs(1);
   }
   const long long t0 = startedAt;

   // connect() must be waiting before the signal
   while (nowMs() - t0 < BLOCK_CHECK_MILLIS && !finished) {
      sleepMs(5);
   }
   if (finished) {
      return abandonInconclusive("the connection attempt ended before any signal was sent: connect() did not block");
   }

   // the signal, and proof that it ran in the connecting thread
   const int before = g_handlerCount;
   ::pthread_kill(worker.native_handle(), SIGUSR1);
   {
      const long long limit = nowMs() + 300;
      while (g_handlerCount == before && nowMs() < limit) {
         sleepMs(2);
      }
   }
   pthread_t handlerThread = g_handlerThread;
   if (g_handlerCount == before || !pthread_equal(handlerThread, worker.native_handle())) {
      return abandonInconclusive("the signal handler did not run in the thread that is connecting");
   }

   for (int i = 0; i < extraSignals; ++i) {
      const long long at = t0 + 600 + 300 * i;
      while (nowMs() < at && !finished) {
         sleepMs(5);
      }
      if (!finished) {
         ::pthread_kill(worker.native_handle(), SIGUSR1);
      }
   }

   const long long limit = t0 + SAFETY_MILLIS;
   if (ending == END_WAIT_FOR_RETURN) {
      while (!finished && nowMs() < limit) {
         sleepMs(5);
      }
   } else {
      while (nowMs() - t0 < RELIEVE_AT_MILLIS && !finished) {
         sleepMs(5);
      }
      if (ending == END_RELIEVE) {
         relieve();
      } else {
         for (int fd : fillers) { ::close(fd); }
         fillers.clear();
         ::close(listenFd);
         listenFd = -1;
      }
      while (!finished && nowMs() < limit) {
         if (ending == END_RELIEVE) {
            serve();
         }
         sleepMs(10);
      }
   }

   if (!finished) {
      fprintf(note, "the Socket constructor was still waiting %lld ms after it started", SAFETY_MILLIS);
      fflush(note);
      ::_exit(CHILD_CHECK_FAILED);
   }
   wrapUp();
   return CHILD_OK;
}

//******************************************************************************
// the checks, each run in a child

int fail(FILE* note, const char* what, const Outcome& o) {
   fprintf(note, "%s [constructed=%d threw=%d what='%s' errno=%d(%s) polls=%d injected=%d exchanged=%d fd %d->%d]",
           what, o.constructed.load() ? 1 : 0, o.threw ? 1 : 0, o.what.c_str(), o.errnoAtCatch,
           (o.errnoAtCatch != 0) ? strerror(o.errnoAtCatch) : "-", o.polls, o.injected,
           o.exchanged ? 1 : 0, o.fdBefore, o.fdAfter);
   return CHILD_CHECK_FAILED;
}

// A: interrupted, then completes
int checkInterruptedConnectionSucceeds(FILE* note) {
   Outcome o;
   const int rc = runInterruptedConnect(note, END_RELIEVE, &COUNTING_OPS, 2, o);
   if (rc != CHILD_OK) {
      return rc;
   }
   if (o.threw) {
      return fail(note, "the interrupted connection was abandoned: the Socket constructor threw", o);
   }
   if (o.polls == 0) {
      fprintf(note, "the connection completed, but connect() was never interrupted inside Chaudiere (the wait for completion was never entered)");
      return CHILD_INCONCLUSIVE;
   }
   if (!o.constructed || !o.exchanged) {
      return fail(note, "the connection did not carry data", o);
   }
   if (o.fdAfter != o.fdBefore) {
      return fail(note, "a descriptor leaked", o);
   }
   fprintf(note, "interrupted inside Chaudiere (%d poll call(s), %d EINTR from the kernel's poll), completed, data exchanged, no descriptor leaked",
           o.polls, o.realEintr);
   return CHILD_OK;
}

// B: interrupted, then refused
int checkInterruptedConnectionThenRefused(FILE* note) {
   Outcome o;
   const int rc = runInterruptedConnect(note, END_REFUSE, &COUNTING_OPS, 0, o);
   if (rc != CHILD_OK) {
      return rc;
   }
   if (o.constructed) {
      return fail(note, "a refused connection was reported as established", o);
   }
   if (o.polls == 0) {
      if (o.errnoAtCatch == EINTR) {
         return fail(note, "the interrupted connection was abandoned as a failure (EINTR)", o);
      }
      fprintf(note, "the attempt was refused without connect() having been interrupted inside Chaudiere");
      return CHILD_INCONCLUSIVE;
   }
   if (!o.threw || o.notBasicException || o.what != "Unable to open socket") {
      return fail(note, "the failure was not reported through BasicException(\"Unable to open socket\")", o);
   }
   if (o.errnoAtCatch != ECONNREFUSED) {
      return fail(note, "the error code was not preserved (expected ECONNREFUSED)", o);
   }
   if (o.fdAfter != o.fdBefore) {
      return fail(note, "a descriptor leaked", o);
   }
   fprintf(note, "interrupted inside Chaudiere (%d poll call(s)), then refused: BasicException \"Unable to open socket\", errno ECONNREFUSED, no descriptor leaked",
           o.polls);
   return CHILD_OK;
}

// C: an ordinary refused connection
int checkOrdinaryRefusedConnection(FILE* note) {
   const int port = unusedPort();
   if (port < 0) {
      fprintf(note, "could not find an unused port");
      return CHILD_SETUP_FAILED;
   }
   resetCounters();
   detail::setConnectIoOps(&COUNTING_OPS);

   try { Socket warmUp("127.0.0.1", port); } catch (const BasicException&) {}
   resetCounters();

   const int before = lowestFreeDescriptor();
   bool threw = false;
   bool basic = false;
   std::string what;
   int error = 0;
   try {
      Socket socket("127.0.0.1", port);
   } catch (const BasicException& be) {
      error = errno;
      threw = true;
      basic = true;
      what = be.whatString();
   } catch (...) {
      error = errno;
      threw = true;
   }
   const int after = lowestFreeDescriptor();
   const int polls = g_pollCalls;
   detail::setConnectIoOps(nullptr);

   if (!threw || !basic || what != "Unable to open socket") {
      fprintf(note, "the constructor did not throw BasicException(\"Unable to open socket\") (threw=%d basic=%d what='%s')",
              threw ? 1 : 0, basic ? 1 : 0, what.c_str());
      return CHILD_CHECK_FAILED;
   }
   if (error != ECONNREFUSED) {
      fprintf(note, "errno was %d (%s), expected ECONNREFUSED", error, strerror(error));
      return CHILD_CHECK_FAILED;
   }
   if (polls != 0) {
      fprintf(note, "the wait for an interrupted connection was entered %d time(s) without any interruption", polls);
      return CHILD_CHECK_FAILED;
   }
   if (after != before) {
      fprintf(note, "a descriptor leaked (lowest free %d -> %d)", before, after);
      return CHILD_CHECK_FAILED;
   }
   fprintf(note, "BasicException \"Unable to open socket\", errno ECONNREFUSED, recovery path not entered, no descriptor leaked");
   return CHILD_OK;
}

// D: an ordinary successful connection
int checkUninterruptedConnection(FILE* note) {
   const int before = lowestFreeDescriptor();      // before anything of the check is open
   int port = 0;
   const int listenFd = listenLoopback(4, port);
   if (listenFd < 0) {
      fprintf(note, "could not create a listener");
      return CHILD_SETUP_FAILED;
   }
   setNonBlocking(listenFd, false);
   resetCounters();
   detail::setConnectIoOps(&COUNTING_OPS);

   bool threw = false;
   bool exchanged = false;
   bool connected = false;
   {
      try {
         Socket socket("127.0.0.1", port);
         connected = socket.isConnected();
         const int server = ::accept(listenFd, nullptr, nullptr);
         if (server >= 0) {
            std::thread answerer([server]() { answerPing(server); });
            exchanged = pingPong(socket);
            answerer.join();
            ::close(server);
         }
      } catch (...) {
         threw = true;
      }
   }
   const int polls = g_pollCalls;
   detail::setConnectIoOps(nullptr);
   ::close(listenFd);
   const int after = lowestFreeDescriptor();

   if (threw || !connected) {
      fprintf(note, "an ordinary connection failed (threw=%d connected=%d)", threw ? 1 : 0, connected ? 1 : 0);
      return CHILD_CHECK_FAILED;
   }
   if (polls != 0) {
      fprintf(note, "the wait for an interrupted connection was entered %d time(s) for an uninterrupted connect()", polls);
      return CHILD_CHECK_FAILED;
   }
   if (!exchanged) {
      fprintf(note, "the connection carried no data");
      return CHILD_CHECK_FAILED;
   }
   if (after != before) {
      fprintf(note, "a descriptor leaked (lowest free %d -> %d)", before, after);
      return CHILD_CHECK_FAILED;
   }
   fprintf(note, "connected without entering the recovery path, data exchanged, no descriptor leaked");
   return CHILD_OK;
}

// E: poll() itself is interrupted again and again, then the connection completes
int checkRepeatedPollInterruptions(FILE* note) {
   g_injectEintr = 3;
   Outcome o;
   const int rc = runInterruptedConnect(note, END_RELIEVE, &EINTR_THEN_REAL_OPS, 0, o);
   if (rc != CHILD_OK) {
      return rc;
   }
   if (o.threw) {
      return fail(note, "the connection was abandoned after poll() was interrupted", o);
   }
   if (o.injected != 3 || o.polls < 4) {
      return fail(note, "poll() was not resumed after each interruption (expected 3 injected EINTR and at least 4 calls)", o);
   }
   if (!o.constructed || !o.exchanged) {
      return fail(note, "the connection did not carry data", o);
   }
   if (o.fdAfter != o.fdBefore) {
      return fail(note, "a descriptor leaked", o);
   }
   fprintf(note, "poll() interrupted %d times, resumed, %d calls in all, completed, data exchanged, no descriptor leaked",
           o.injected, o.polls);
   return CHILD_OK;
}

// F: poll() fails otherwise, or reports nothing useful: the recovery ends in the connection's failure
int checkPollFailure(FILE* note, const detail::SocketIoOps* ops, int expectedErrno, int maxPolls, const char* what) {
   Outcome o;
   const int rc = runInterruptedConnect(note, END_WAIT_FOR_RETURN, ops, 0, o);
   if (rc != CHILD_OK) {
      return rc;
   }
   if (o.constructed) {
      return fail(note, "a connection whose wait failed was reported as established", o);
   }
   if (!o.threw || o.notBasicException || o.what != "Unable to open socket") {
      return fail(note, "the failure was not reported through BasicException(\"Unable to open socket\")", o);
   }
   if (o.errnoAtCatch != expectedErrno) {
      return fail(note, "the error code was not the one of the failure", o);
   }
   if (o.polls < 1 || o.polls > maxPolls) {
      return fail(note, "the number of poll() calls shows that the recovery did not end at once", o);
   }
   if (o.fdAfter != o.fdBefore) {
      return fail(note, "a descriptor leaked", o);
   }
   fprintf(note, "%s: BasicException \"Unable to open socket\", errno %d (%s), %d poll call(s), no retry loop, no descriptor leaked",
           what, o.errnoAtCatch, strerror(o.errnoAtCatch), o.polls);
   return CHILD_OK;
}

int checkPollFailureErrno(FILE* note) {
   g_failErrno = EIO;
   return checkPollFailure(note, &FAILING_OPS, EIO, 1, "poll() failed with EIO");
}

int checkPollInvalidDescriptor(FILE* note) {
   return checkPollFailure(note, &INVALID_DESCRIPTOR_OPS, EBADF, 1, "poll() reported POLLNVAL");
}

int checkPollMeaninglessResults(FILE* note) {
   return checkPollFailure(note, &SILENT_OPS, ETIMEDOUT, 16, "poll() kept returning no event");
}

// a connected / refused non-blocking attempt, for the unit checks of awaitConnectCompletion()
int startNonBlockingConnect(int port) {
   const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0) {
      return -1;
   }
   setNonBlocking(fd, true);
   sockaddr_in addr = loopbackAddress(port);
   if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 && errno != EINPROGRESS) {
      // an immediate refusal is a valid outcome of a loopback connect; the error is still pending
   }
   return fd;
}

// waits until the attempt has ended without reading its outcome
void waitUntilSettled(int fd) {
   pollfd p;
   p.fd = fd;
   p.events = POLLOUT;
   p.revents = 0;
   ::poll(&p, 1, 3000);
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
// A - F: real interrupted connections

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testInterruptedConnectionSucceeds) {
   const ChildResult r = runInChild(checkInterruptedConnectionSucceeds, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("interrupted connection completes", r);
}

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testInterruptedConnectionThenRefused) {
   const ChildResult r = runInChild(checkInterruptedConnectionThenRefused, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("interrupted connection is refused", r);
}

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testOrdinaryRefusedConnection) {
   const ChildResult r = runInChild(checkOrdinaryRefusedConnection, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("ordinary refused connection", r);
}

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testUninterruptedConnection) {
   const ChildResult r = runInChild(checkUninterruptedConnection, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("uninterrupted connection", r);
}

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testRepeatedPollInterruptionsAreResumed) {
   const ChildResult r = runInChild(checkRepeatedPollInterruptions, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("repeated poll() interruptions", r);
}

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testPollFailureEndsTheRecovery) {
   const ChildResult r = runInChild(checkPollFailureErrno, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("poll() failure", r);
}

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testPollInvalidDescriptorEndsTheRecovery) {
   const ChildResult r = runInChild(checkPollInvalidDescriptor, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("poll() POLLNVAL", r);
}

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testMeaninglessPollResultsAreBounded) {
   const ChildResult r = runInChild(checkPollMeaninglessResults, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("meaningless poll() results", r);
}

//******************************************************************************
// the outcome reader on its own (no signals): what awaitConnectCompletion() makes of each outcome

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testEstablishedAndRefusedAttemptsAreTold) {
   int port = 0;
   const int listenFd = listenLoopback(4, port);
   require(listenFd >= 0, "a loopback listener");

   const int established = startNonBlockingConnect(port);
   require(established >= 0, "a connecting socket");
   require(detail::awaitConnectCompletion(established, detail::systemSocketIoOps()) == 0,
           "an attempt that completed is reported as established (0)");
   require(socketErrorOf(established) == 0, "SO_ERROR is clear after an established attempt");
   ::close(established);
   ::close(listenFd);

   const int refusedPort = unusedPort();
   require(refusedPort > 0, "an unused port");
   const int refused = startNonBlockingConnect(refusedPort);
   require(refused >= 0, "a connecting socket");
   require(detail::awaitConnectCompletion(refused, detail::systemSocketIoOps()) == ECONNREFUSED,
           "a refused attempt is reported with its SO_ERROR (ECONNREFUSED)");
   ::close(refused);
}

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testWritabilityAloneIsNotSuccess) {
   const int refusedPort = unusedPort();
   require(refusedPort > 0, "an unused port");
   const int refused = startNonBlockingConnect(refusedPort);
   require(refused >= 0, "a connecting socket");
   waitUntilSettled(refused);

   g_pollCalls = 0;
   // poll() claims plain writability for a socket whose connection was refused
   const int result = detail::awaitConnectCompletion(refused, WRITABLE_ONLY_OPS);
   require(result == ECONNREFUSED, "POLLOUT alone is not read as success: SO_ERROR (ECONNREFUSED) decides");
   require(g_pollCalls == 1, "one poll() call was enough");
   ::close(refused);
}

POIVRE_TEST_CASE(TestSocketConnectInterrupt, testHangupWithoutErrorIsNotFailure) {
   int port = 0;
   const int listenFd = listenLoopback(4, port);
   require(listenFd >= 0, "a loopback listener");
   const int connected = startNonBlockingConnect(port);
   require(connected >= 0, "a connecting socket");
   waitUntilSettled(connected);

   g_pollCalls = 0;
   // poll() reports POLLHUP together with POLLOUT, but the attempt itself succeeded
   const int result = detail::awaitConnectCompletion(connected, HANGUP_OPS);
   require(result == 0, "POLLHUP does not reject a connection whose SO_ERROR is clear");
   ::close(connected);
   ::close(listenFd);
}
