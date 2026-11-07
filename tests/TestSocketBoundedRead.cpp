// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

// Tests of Socket::readSocket(buffer, bytesToRead, timeoutMillis).
//
// Real sockets: every test that has a peer uses a TCP connection over the
// loopback interface on a kernel-chosen port (bind to port 0), so no fixed port
// is involved and nothing can collide with another test or another run.
//
// Real signals: the EINTR-during-poll tests deliver SIGUSR1 to the reading
// thread every couple of milliseconds, with a handler installed without
// SA_RESTART. poll() is never restarted after a handler, so it does return
// EINTR here.
//
// Injected system calls: the kernel cannot be made to return EINTR from a
// MSG_DONTWAIT recv(), nor EAGAIN from recv() right after poll() reported the
// descriptor readable, on demand. Those cases (and error returns) call the same
// loop with scripted poll()/recv() results (chaudiere::detail::SocketIoOps,
// src/internal/SocketIo.h) - see the section marked SCRIPTED. One further case
// uses the real recv() behind a poll() that always claims readiness, to show on
// a real kernel that the receive cannot block.
//
// Bounded execution: every test case runs under a HangGuard. If a regression
// makes the code block, the guard ends the process after HANG_LIMIT_SECONDS
// with a message naming the test, instead of hanging the whole run.

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "TestSocketBoundedRead.h"
#include "Socket.h"
#include "internal/SocketIo.h"

using namespace chaudiere;

//******************************************************************************

TestSocketBoundedRead::TestSocketBoundedRead() :
   poivre::TestSuite("TestSocketBoundedRead") {
}

//******************************************************************************

namespace {

typedef std::chrono::steady_clock Clock;

// a test that has not finished after this long is considered hung
constexpr int HANG_LIMIT_SECONDS = 60;

// The budget used by most tests, and how late a timed-out call may return
// before it counts as "did not honour the budget". The bound is deliberately
// generous (loaded CI, sanitizers): a design that restarts the budget on
// progress or never times out misses it by seconds, not milliseconds.
constexpr int BUDGET = 300;
constexpr int SLACK = 1500;

long long millisSince(Clock::time_point since) {
   return std::chrono::duration_cast<std::chrono::milliseconds>(
      Clock::now() - since).count();
}

std::string fmt(const char* what, long long value) {
   return std::string(what) + " (" + std::to_string(value) + ")";
}

// Waits on a condition variable until pred() holds or `millis` milliseconds have
// passed, and returns pred(). The absolute time is on the system clock on purpose:
// libstdc++ waits on a steady_clock time with pthread_cond_clockwait, which GCC 11's
// ThreadSanitizer does not intercept, so it then reports lock and data races that
// do not exist. Only the length of a wait matters here, so a step of the system
// clock could at worst shorten or lengthen one wait in a test.
template <class Predicate>
bool waitForMillis(std::condition_variable& condition,
                   std::unique_lock<std::mutex>& lock,
                   long long millis,
                   Predicate pred) {
   return condition.wait_until(lock,
                               std::chrono::system_clock::now() +
                                  std::chrono::milliseconds(millis),
                               pred);
}

//******************************************************************************
// bounded execution

class HangGuard
{
public:
   explicit HangGuard(const char* testName) :
      m_testName(testName),
      m_finished(false),
      m_watchdog([this] { watch(); }) {
   }

   ~HangGuard() {
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_finished = true;
      }
      m_condition.notify_all();
      m_watchdog.join();
   }

private:
   void watch() {
      std::unique_lock<std::mutex> lock(m_mutex);
      if (!waitForMillis(m_condition,
                         lock,
                         HANG_LIMIT_SECONDS * 1000LL,
                         [this] { return m_finished; })) {
         fprintf(stderr,
                 "\n*** HANG: TestSocketBoundedRead.%s did not finish within %d seconds\n",
                 m_testName,
                 HANG_LIMIT_SECONDS);
         fflush(stderr);
         _exit(97);
      }
   }

   const char* m_testName;
   std::mutex m_mutex;
   std::condition_variable m_condition;
   bool m_finished;
   std::thread m_watchdog;   // last: started by the constructor
};

//******************************************************************************
// loopback TCP connection on a kernel-chosen port

class TcpPair
{
public:
   TcpPair() : m_client(-1), m_server(-1) {
      const int listener = ::socket(AF_INET, SOCK_STREAM, 0);
      if (listener < 0) {
         return;
      }

      struct sockaddr_in addr;
      ::memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      addr.sin_port = 0;   // the kernel picks

      socklen_t addrLen = sizeof(addr);
      if (::bind(listener, (struct sockaddr*) &addr, sizeof(addr)) == 0 &&
          ::listen(listener, 1) == 0 &&
          ::getsockname(listener, (struct sockaddr*) &addr, &addrLen) == 0) {
         const int client = ::socket(AF_INET, SOCK_STREAM, 0);
         if (client >= 0) {
            if (::connect(client, (struct sockaddr*) &addr, sizeof(addr)) == 0) {
               m_server = ::accept(listener, nullptr, nullptr);
               m_client = client;
            } else {
               ::close(client);
            }
         }
      }
      ::close(listener);

      if (m_server < 0 && m_client >= 0) {
         ::close(m_client);
         m_client = -1;
      }
   }

   ~TcpPair() {
      closeClient();
      closeServer();
   }

   TcpPair(const TcpPair&) = delete;
   TcpPair& operator=(const TcpPair&) = delete;

   bool isOpen() const { return m_client >= 0 && m_server >= 0; }

   int client() const { return m_client; }
   int server() const { return m_server; }

   // hands the client descriptor to a Socket, which closes it
   int takeClient() {
      const int fd = m_client;
      m_client = -1;
      return fd;
   }

   void closeClient() {
      if (m_client >= 0) {
         ::close(m_client);
         m_client = -1;
      }
   }

   void closeServer() {
      if (m_server >= 0) {
         ::close(m_server);
         m_server = -1;
      }
   }

private:
   int m_client;
   int m_server;
};

#ifdef MSG_NOSIGNAL
constexpr int SEND_FLAGS = MSG_NOSIGNAL;
#else
constexpr int SEND_FLAGS = 0;
#endif

bool sendAll(int fd, const std::string& bytes) {
   size_t sent = 0;
   while (sent < bytes.size()) {
      const ssize_t n = ::send(fd, bytes.data() + sent, bytes.size() - sent, SEND_FLAGS);
      if (n <= 0) {
         return false;
      }
      sent += (size_t) n;
   }
   return true;
}

// waits until at least `count` bytes can be read from fd without blocking
bool waitAvailable(int fd, int count) {
   const Clock::time_point start = Clock::now();
   while (millisSince(start) < 5000) {
      int available = 0;
      if (::ioctl(fd, FIONREAD, &available) == 0 && available >= count) {
         return true;
      }
      ::usleep(1000);
   }
   return false;
}

// bytes the peer sends: `atMillis` after the Peer was created
struct Step
{
   int atMillis;
   std::string bytes;
};

// Plays a script of sends on one end of a connection from its own thread, then
// either closes that end or holds it open (silent) until destroyed. Declare it
// after the Socket under test so that it is stopped first.
class Peer
{
public:
   Peer(int fd, const std::vector<Step>& steps, bool closeWhenDone) :
      m_fd(fd),
      m_steps(steps),
      m_closeWhenDone(closeWhenDone),
      m_stop(false),
      m_start(Clock::now()),
      m_thread([this] { play(); }) {
   }

   ~Peer() {
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_stop = true;
      }
      m_condition.notify_all();
      m_thread.join();
   }

   Peer(const Peer&) = delete;
   Peer& operator=(const Peer&) = delete;

private:
   // true if `when` was reached, false if the Peer was stopped first
   bool waitUntil(Clock::time_point when) {
      const long long millis = std::chrono::duration_cast<std::chrono::milliseconds>(
         when - Clock::now()).count();
      std::unique_lock<std::mutex> lock(m_mutex);
      return !waitForMillis(m_condition, lock, (millis > 0) ? millis : 0,
                            [this] { return m_stop; });
   }

   void play() {
      for (const Step& step : m_steps) {
         if (!waitUntil(m_start + std::chrono::milliseconds(step.atMillis))) {
            return;
         }
         if (!sendAll(m_fd, step.bytes)) {
            return;
         }
      }

      if (m_closeWhenDone) {
         ::shutdown(m_fd, SHUT_WR);
      } else {
         // silent: hold the connection open until the test is done
         std::unique_lock<std::mutex> lock(m_mutex);
         m_condition.wait(lock, [this] { return m_stop; });
      }
   }

   int m_fd;
   std::vector<Step> m_steps;
   bool m_closeWhenDone;
   std::mutex m_mutex;
   std::condition_variable m_condition;
   bool m_stop;
   Clock::time_point m_start;
   std::thread m_thread;   // last: started by the constructor
};

// setLineInputBuffer() is protected on Socket
class PrimableSocket : public Socket
{
public:
   explicit PrimableSocket(int fd) : Socket(fd) {}
   using Socket::setLineInputBuffer;
};

std::string bytesOf(const char* buffer, int count) {
   return std::string(buffer, (size_t) (count > 0 ? count : 0));
}

// the "0123456789" pattern, `count` bytes long, starting at digit `first`
std::string pattern(int count, int first = 0) {
   std::string s;
   for (int i = 0; i < count; ++i) {
      s += (char) ('0' + ((first + i) % 10));
   }
   return s;
}

//******************************************************************************
// SCRIPTED: poll()/recv() whose results the test decides

struct ScriptedRecv
{
   ssize_t result;     // > 0: bytes of `data` to deliver; 0: end of stream; < 0: failure
   int error;          // errno for a failure
   std::string data;
};

struct ScriptedPoll
{
   int result;         // > 0: readable; 0: timed out; < 0: failure
   int error;
};

std::vector<ScriptedPoll> g_pollScript;      // consumed in order; then "readable"
std::vector<ScriptedRecv> g_recvScript;      // consumed in order; then EAGAIN
size_t g_pollIndex = 0;
size_t g_recvIndex = 0;
std::vector<int> g_pollTimeouts;             // the timeout argument of every poll() call
int g_recvCalls = 0;
bool g_everyRecvWasDontWait = true;
bool g_pollAlwaysReadyWhenScriptEnds = true;

void resetScript() {
   g_pollScript.clear();
   g_recvScript.clear();
   g_pollIndex = 0;
   g_recvIndex = 0;
   g_pollTimeouts.clear();
   g_recvCalls = 0;
   g_everyRecvWasDontWait = true;
   g_pollAlwaysReadyWhenScriptEnds = true;
}

int scriptedPoll(struct pollfd* fds, nfds_t, int timeoutMillis) {
   g_pollTimeouts.push_back(timeoutMillis);

   if (g_pollIndex < g_pollScript.size()) {
      const ScriptedPoll step = g_pollScript[g_pollIndex++];
      if (step.result < 0) {
         errno = step.error;
         return -1;
      }
      fds[0].revents = (step.result > 0) ? POLLIN : 0;
      return step.result;
   }

   if (g_pollAlwaysReadyWhenScriptEnds) {
      fds[0].revents = POLLIN;
      return 1;
   }
   fds[0].revents = 0;
   return 0;
}

ssize_t scriptedRecv(int, void* buffer, size_t length, int flags) {
   ++g_recvCalls;
   if ((flags & MSG_DONTWAIT) == 0) {
      g_everyRecvWasDontWait = false;
   }

   if (g_recvIndex < g_recvScript.size()) {
      const ScriptedRecv& step = g_recvScript[g_recvIndex++];
      if (step.result > 0) {
         const size_t n = (step.data.size() < length) ? step.data.size() : length;
         ::memcpy(buffer, step.data.data(), n);
         return (ssize_t) n;
      }
      if (step.result == 0) {
         return 0;
      }
      errno = step.error;
      return -1;
   }

   errno = EAGAIN;
   return -1;
}

const detail::SocketIoOps SCRIPTED_OPS = { scriptedPoll, scriptedRecv };

// a real recv() behind a poll() that always claims the descriptor is readable
int alwaysReadyPoll(struct pollfd* fds, nfds_t, int) {
   fds[0].revents = POLLIN;
   return 1;
}

ssize_t realRecv(int fd, void* buffer, size_t length, int flags) {
   return ::recv(fd, buffer, length, flags);
}

const detail::SocketIoOps LYING_POLL_REAL_RECV_OPS = { alwaysReadyPoll, realRecv };

// the real calls, counting how often poll() was interrupted and called
int g_countedPolls = 0;
int g_countedInterrupts = 0;

int countingPoll(struct pollfd* fds, nfds_t nfds, int timeoutMillis) {
   const int result = ::poll(fds, nfds, timeoutMillis);
   const int savedErrno = errno;
   ++g_countedPolls;
   if (result < 0 && savedErrno == EINTR) {
      ++g_countedInterrupts;
   }
   errno = savedErrno;
   return result;
}

const detail::SocketIoOps COUNTING_OPS = { countingPoll, realRecv };

//******************************************************************************
// signals

volatile sig_atomic_t g_signalsHandled = 0;

void onStormSignal(int) {
   g_signalsHandled = g_signalsHandled + 1;
}

// Delivers SIGUSR1 to the thread that constructed it every few milliseconds,
// with a handler installed WITHOUT SA_RESTART, until destroyed.
class SignalStorm
{
public:
   explicit SignalStorm(int intervalMillis) :
      m_target(pthread_self()),
      m_intervalMillis(intervalMillis),
      m_stop(false),
      m_installed(false) {

      g_signalsHandled = 0;

      struct sigaction action;
      ::memset(&action, 0, sizeof(action));
      action.sa_handler = onStormSignal;
      ::sigemptyset(&action.sa_mask);
      action.sa_flags = 0;   // deliberately no SA_RESTART
      m_installed = (::sigaction(SIGUSR1, &action, &m_previous) == 0);

      if (m_installed) {
         m_thread = std::thread([this] { run(); });
      }
   }

   ~SignalStorm() {
      m_stop = true;
      if (m_thread.joinable()) {
         m_thread.join();
      }
      if (m_installed) {
         ::sigaction(SIGUSR1, &m_previous, nullptr);
      }
   }

   SignalStorm(const SignalStorm&) = delete;
   SignalStorm& operator=(const SignalStorm&) = delete;

   bool isInstalled() const { return m_installed; }

private:
   void run() {
      while (!m_stop) {
         ::pthread_kill(m_target, SIGUSR1);
         std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMillis));
      }
   }

   pthread_t m_target;
   int m_intervalMillis;
   std::atomic<bool> m_stop;
   bool m_installed;
   struct sigaction m_previous;
   std::thread m_thread;
};

}

//******************************************************************************
// 1. a silent peer

POIVRE_TEST_CASE(TestSocketBoundedRead, testSilentPeerTimesOut) {
   HangGuard guard("testSilentPeerTimesOut");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Socket socket(pair.takeClient());
   Peer peer(pair.server(), {}, false);

   char buffer[16];
   const Clock::time_point start = Clock::now();
   const int result = socket.readSocket(buffer, sizeof(buffer), BUDGET);
   const long long elapsed = millisSince(start);

   require(result == -1, fmt("no bytes arrived: -1", result));
   require(elapsed >= BUDGET * 9 / 10, fmt("waited for the budget", elapsed));
   require(elapsed <= BUDGET + SLACK, fmt("returned soon after the budget", elapsed));
   require(socket.isConnected(), "a timeout does not close the socket");
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testSilentPeerDoesNotBusyLoop) {
   HangGuard guard("testSilentPeerDoesNotBusyLoop");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Peer peer(pair.server(), {}, false);

   g_countedPolls = 0;
   char buffer[16];
   const int result = detail::boundedReadFromDescriptor(pair.client(),
                                                        buffer,
                                                        sizeof(buffer),
                                                        BUDGET,
                                                        Clock::now(),
                                                        COUNTING_OPS);

   require(result == 0, fmt("no bytes arrived: 0", result));
   // one poll() waits the whole budget (a second may follow a slightly early
   // wake-up). A loop that spun would make thousands of calls.
   require(g_countedPolls >= 1 && g_countedPolls <= 4,
           fmt("poll() calls while waiting 300 ms", g_countedPolls));
}

//******************************************************************************
// 2./3. partial data followed by silence

POIVRE_TEST_CASE(TestSocketBoundedRead, testPartialPrefixThenSilenceReturnsShortCount) {
   HangGuard guard("testPartialPrefixThenSilenceReturnsShortCount");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Socket socket(pair.takeClient());
   // 4 of the 10 bytes at once, the other 6 only well after the budget
   Peer peer(pair.server(), { {0, "0123"}, {BUDGET * 2, "456789"} }, false);

   char buffer[10];
   const Clock::time_point start = Clock::now();
   const int result = socket.readSocket(buffer, sizeof(buffer), BUDGET);
   const long long elapsed = millisSince(start);

   require(result == 4, fmt("short count: the 4 bytes that arrived", result));
   requireStringEquals("0123", bytesOf(buffer, 4), "the bytes that arrived");
   require(elapsed >= BUDGET * 9 / 10, fmt("waited for the budget", elapsed));
   require(elapsed <= BUDGET + SLACK, fmt("returned soon after the budget", elapsed));

   // nothing was lost and the socket is intact: the rest can still be read
   char rest[6];
   const int second = socket.readSocket(rest, sizeof(rest), 5000);
   require(second == 6, fmt("the late bytes are still readable", second));
   requireStringEquals("456789", bytesOf(rest, 6), "the late bytes");
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testPartialPayloadThenSilenceReturnsShortCount) {
   HangGuard guard("testPartialPayloadThenSilenceReturnsShortCount");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Socket socket(pair.takeClient());
   Peer peer(pair.server(), { {0, pattern(60)} }, false);

   char buffer[100];
   const Clock::time_point start = Clock::now();
   const int result = socket.readSocket(buffer, sizeof(buffer), BUDGET);
   const long long elapsed = millisSince(start);

   require(result == 60, fmt("short count: the 60 bytes that arrived", result));
   requireStringEquals(pattern(60), bytesOf(buffer, 60), "the bytes that arrived");
   require(elapsed >= BUDGET * 9 / 10, fmt("waited for the budget", elapsed));
   require(elapsed <= BUDGET + SLACK, fmt("returned soon after the budget", elapsed));
}

//******************************************************************************
// 4. slow trickle: the budget is not restarted by progress

POIVRE_TEST_CASE(TestSocketBoundedRead, testSlowTrickleIsCutOffAtTheDeadline) {
   HangGuard guard("testSlowTrickleIsCutOffAtTheDeadline");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   // one byte every 30 ms, so every gap is a tenth of the budget; all 40 bytes
   // would take 1.2 s. An inactivity timeout of the same length would wait for
   // all of them.
   std::vector<Step> steps;
   for (int i = 0; i < 40; ++i) {
      steps.push_back({i * 30, std::string(1, (char) ('0' + (i % 10)))});
   }

   Socket socket(pair.takeClient());
   Peer peer(pair.server(), steps, false);

   char buffer[40];
   const Clock::time_point start = Clock::now();
   const int result = socket.readSocket(buffer, sizeof(buffer), BUDGET);
   const long long elapsed = millisSince(start);

   require(result >= 1 && result < 40, fmt("cut short, with the bytes that arrived", result));
   requireStringEquals(pattern(result), bytesOf(buffer, result), "bytes in order");
   require(elapsed >= BUDGET * 9 / 10, fmt("waited for the budget", elapsed));
   require(elapsed <= BUDGET + SLACK, fmt("returned soon after the budget", elapsed));
}

//******************************************************************************
// 5. valid segmented delivery within the budget

POIVRE_TEST_CASE(TestSocketBoundedRead, testSegmentedDeliveryWithinBudgetSucceeds) {
   HangGuard guard("testSegmentedDeliveryWithinBudgetSucceeds");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Socket socket(pair.takeClient());
   Peer peer(pair.server(),
             { {0, pattern(20, 0)}, {20, pattern(20, 0)}, {40, pattern(20, 0)},
               {60, pattern(20, 0)}, {80, pattern(20, 0)} },
             false);

   char buffer[100];
   const Clock::time_point start = Clock::now();
   const int result = socket.readSocket(buffer, sizeof(buffer), 5000);
   const long long elapsed = millisSince(start);

   require(result == 100, fmt("the whole read succeeded", result));
   requireStringEquals(pattern(20) + pattern(20) + pattern(20) + pattern(20) + pattern(20),
                       bytesOf(buffer, 100),
                       "bytes in order");
   require(elapsed < 3000, fmt("did not wait out the budget", elapsed));
}

//******************************************************************************
// 6. the deadline spans several segments

POIVRE_TEST_CASE(TestSocketBoundedRead, testDeadlineSpansMultipleSegments) {
   HangGuard guard("testDeadlineSpansMultipleSegments");

   const int budget = 600;

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Socket socket(pair.takeClient());
   // segments of 10 bytes. Every gap (270 ms) is well under the budget, but the
   // fourth segment (810 ms) only arrives after it: a budget restarted by each
   // segment would collect all 40 bytes.
   Peer peer(pair.server(),
             { {0, pattern(10, 0)}, {270, pattern(10, 10)}, {540, pattern(10, 20)},
               {810, pattern(10, 30)} },
             false);

   char buffer[40];
   const Clock::time_point start = Clock::now();
   const int result = socket.readSocket(buffer, sizeof(buffer), budget);
   const long long elapsed = millisSince(start);

   require(result >= 10 && result < 40,
           fmt("cut short after some segments, never complete", result));
   require(result % 10 == 0, fmt("whole segments only", result));
   requireStringEquals(pattern(result), bytesOf(buffer, result), "bytes in order");
   require(elapsed >= budget * 9 / 10, fmt("waited for the budget", elapsed));
   require(elapsed <= budget + SLACK, fmt("returned soon after the budget", elapsed));
}

//******************************************************************************
// 7./8. EINTR while waiting for readiness

POIVRE_TEST_CASE(TestSocketBoundedRead, testSignalsDuringPollDoNotFailAValidRead) {
   HangGuard guard("testSignalsDuringPollDoNotFailAValidRead");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Socket socket(pair.takeClient());
   // 8 segments over about 280 ms
   std::vector<Step> steps;
   for (int i = 0; i < 8; ++i) {
      steps.push_back({i * 40, pattern(10)});
   }
   Peer peer(pair.server(), steps, false);

   {
      SignalStorm storm(2);
      require(storm.isInstalled(), "signal handler installed");

      char buffer[80];
      const int result = socket.readSocket(buffer, sizeof(buffer), 10000);

      require(result == 80, fmt("signals did not fail the read", result));
      std::string expected;
      for (int i = 0; i < 8; ++i) {
         expected += pattern(10);
      }
      requireStringEquals(expected, bytesOf(buffer, 80), "bytes in order");
      require(g_signalsHandled > 0, fmt("signals were delivered to this thread", g_signalsHandled));
   }
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testPollIsInterruptedAndResumed) {
   HangGuard guard("testPollIsInterruptedAndResumed");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   std::vector<Step> steps;
   for (int i = 0; i < 8; ++i) {
      steps.push_back({i * 40, pattern(10)});
   }
   Peer peer(pair.server(), steps, false);

   g_countedPolls = 0;
   g_countedInterrupts = 0;
   {
      SignalStorm storm(2);
      require(storm.isInstalled(), "signal handler installed");

      char buffer[80];
      const int result = detail::boundedReadFromDescriptor(pair.client(),
                                                           buffer,
                                                           sizeof(buffer),
                                                           10000,
                                                           Clock::now(),
                                                           COUNTING_OPS);
      require(result == 80, fmt("the read completed", result));
   }

   // the test is only meaningful if poll() really was interrupted
   require(g_countedInterrupts > 0, fmt("poll() calls that returned EINTR", g_countedInterrupts));
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testSignalsDoNotExtendTheDeadline) {
   HangGuard guard("testSignalsDoNotExtendTheDeadline");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Socket socket(pair.takeClient());
   Peer peer(pair.server(), {}, false);

   char buffer[16];
   Clock::time_point start;
   int result;
   long long elapsed;
   {
      SignalStorm storm(2);
      require(storm.isInstalled(), "signal handler installed");

      start = Clock::now();
      result = socket.readSocket(buffer, sizeof(buffer), BUDGET);
      elapsed = millisSince(start);
   }

   require(result == -1, fmt("silent peer: -1", result));
   require(elapsed >= BUDGET * 9 / 10, fmt("signals did not end the wait early", elapsed));
   require(elapsed <= BUDGET + SLACK, fmt("signals did not extend the wait", elapsed));
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testPollEintrIsResumed_Scripted) {
   HangGuard guard("testPollEintrIsResumed_Scripted");

   resetScript();
   g_pollScript = { {-1, EINTR}, {-1, EINTR}, {-1, EINTR}, {1, 0} };
   g_recvScript = { {5, 0, "abcde"} };

   char buffer[5];
   const int result = detail::boundedReadFromDescriptor(-1, buffer, sizeof(buffer), -1,
                                                        Clock::now(), SCRIPTED_OPS);

   require(result == 5, fmt("the read completed", result));
   requireStringEquals("abcde", bytesOf(buffer, 5), "bytes");
   require(g_pollTimeouts.size() == 4, fmt("poll() called again after each EINTR", (long long) g_pollTimeouts.size()));
}

//******************************************************************************
// SCRIPTED: EINTR from recv(), EAGAIN after readiness, errors

POIVRE_TEST_CASE(TestSocketBoundedRead, testEintrFromRecvIsRetried_Scripted) {
   HangGuard guard("testEintrFromRecvIsRetried_Scripted");

   resetScript();
   g_recvScript = { {-1, EINTR, ""}, {3, 0, "abc"}, {-1, EINTR, ""}, {5, 0, "defgh"} };

   char buffer[8];
   const int result = detail::boundedReadFromDescriptor(-1, buffer, sizeof(buffer), -1,
                                                        Clock::now(), SCRIPTED_OPS);

   require(result == 8, fmt("EINTR is not an error: the read completed", result));
   requireStringEquals("abcdefgh", bytesOf(buffer, 8), "bytes in order");
   require(g_recvCalls == 4, fmt("recv() calls", g_recvCalls));
   require(g_everyRecvWasDontWait, "every recv() used MSG_DONTWAIT");
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testEagainAfterReadinessIsWaitedOut_Scripted) {
   HangGuard guard("testEagainAfterReadinessIsWaitedOut_Scripted");

   // poll() says readable three times, but recv() finds nothing the first two
   resetScript();
   g_recvScript = { {-1, EAGAIN, ""}, {-1, EWOULDBLOCK, ""}, {6, 0, "abcdef"} };

   char buffer[6];
   const int result = detail::boundedReadFromDescriptor(-1, buffer, sizeof(buffer), 5000,
                                                        Clock::now(), SCRIPTED_OPS);

   require(result == 6, fmt("EAGAIN is not an error: the read completed", result));
   requireStringEquals("abcdef", bytesOf(buffer, 6), "bytes");
   require(g_pollTimeouts.size() == 3, fmt("poll() again after each EAGAIN", (long long) g_pollTimeouts.size()));
   require(g_everyRecvWasDontWait, "every recv() used MSG_DONTWAIT");
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testEagainForeverStillEndsAtTheDeadline_Scripted) {
   HangGuard guard("testEagainForeverStillEndsAtTheDeadline_Scripted");

   // readiness that never produces data: the loop must still respect the budget
   resetScript();

   char buffer[4];
   const Clock::time_point start = Clock::now();
   const int result = detail::boundedReadFromDescriptor(-1, buffer, sizeof(buffer), 100,
                                                        start, SCRIPTED_OPS);
   const long long elapsed = millisSince(start);

   require(result == 0, fmt("no bytes: 0", result));
   require(elapsed >= 90, fmt("ran until the budget", elapsed));
   require(elapsed <= 100 + SLACK, fmt("and stopped at the budget", elapsed));
   // each pass pauses for at least a millisecond, so this cannot be a busy loop:
   // without the pause there would be many thousands of calls
   require((long long) g_pollTimeouts.size() <= elapsed + 50,
           fmt("poll() calls while readiness never produced data", (long long) g_pollTimeouts.size()));
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testRecvErrorAfterPartialReadReturnsPartialCount_Scripted) {
   HangGuard guard("testRecvErrorAfterPartialReadReturnsPartialCount_Scripted");

   resetScript();
   g_recvScript = { {3, 0, "abc"}, {-1, ECONNRESET, ""}, {5, 0, "never"} };

   char buffer[8];
   const int result = detail::boundedReadFromDescriptor(-1, buffer, sizeof(buffer), -1,
                                                        Clock::now(), SCRIPTED_OPS);

   require(result == 3, fmt("the bytes read before the error", result));
   requireStringEquals("abc", bytesOf(buffer, 3), "bytes");
   require(g_recvCalls == 2, fmt("no further recv() after the error", g_recvCalls));
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testPollErrorEndsTheRead_Scripted) {
   HangGuard guard("testPollErrorEndsTheRead_Scripted");

   resetScript();
   g_pollScript = { {1, 0}, {-1, EBADF} };
   g_recvScript = { {3, 0, "abc"} };

   char buffer[8];
   const int result = detail::boundedReadFromDescriptor(-1, buffer, sizeof(buffer), -1,
                                                        Clock::now(), SCRIPTED_OPS);

   require(result == 3, fmt("the bytes read before the poll error", result));
   require(g_pollTimeouts.size() == 2, fmt("no further poll() after the error", (long long) g_pollTimeouts.size()));
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testPollTimeoutArgumentIsTheRemainingBudget_Scripted) {
   HangGuard guard("testPollTimeoutArgumentIsTheRemainingBudget_Scripted");

   // poll() "times out" at once every time, so the loop calls it repeatedly until
   // the budget is gone. Each call must be given what is left, never more.
   resetScript();
   g_pollAlwaysReadyWhenScriptEnds = false;

   char buffer[4];
   const Clock::time_point start = Clock::now();
   const int result = detail::boundedReadFromDescriptor(-1, buffer, sizeof(buffer), 100,
                                                        start, SCRIPTED_OPS);
   const long long elapsed = millisSince(start);

   require(result == 0, fmt("no bytes: 0", result));
   require(elapsed >= 90 && elapsed <= 100 + SLACK, fmt("ended at the budget", elapsed));
   require(g_pollTimeouts.size() >= 2, "poll() was called repeatedly");

   bool inRange = true;
   bool nonIncreasing = true;
   for (size_t i = 0; i < g_pollTimeouts.size(); ++i) {
      if (g_pollTimeouts[i] < 0 || g_pollTimeouts[i] > 100) {
         inRange = false;
      }
      if (i > 0 && g_pollTimeouts[i] > g_pollTimeouts[i - 1]) {
         nonIncreasing = false;
      }
   }
   require(inRange, "every wait was within 0..budget");
   require(nonIncreasing, "the wait never grew: the budget was not restarted");

   // the remaining time is rounded up, so a fraction of a millisecond left is
   // waited for with a 1 ms wait and never with a 0 ms one, which would make
   // the last millisecond a busy loop. The only 0 allowed is a final check once
   // the budget is gone.
   size_t zeroWaits = 0;
   for (size_t i = 0; i < g_pollTimeouts.size(); ++i) {
      if (g_pollTimeouts[i] == 0) {
         ++zeroWaits;
      }
   }
   require(zeroWaits <= 1, fmt("polls with a zero wait while budget remained", (long long) zeroWaits));
   require(zeroWaits == 0 || g_pollTimeouts.back() == 0, "a zero wait can only be the last poll");
   require(g_pollTimeouts.front() > 0 && g_pollTimeouts.front() <= 100,
           fmt("first wait is about the whole budget", g_pollTimeouts.front()));
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testPollTimeoutArgumentForUnlimitedAndZero_Scripted) {
   HangGuard guard("testPollTimeoutArgumentForUnlimitedAndZero_Scripted");

   char buffer[4];

   // unlimited: poll() is asked to wait without limit
   resetScript();
   g_recvScript = { {4, 0, "abcd"} };
   int result = detail::boundedReadFromDescriptor(-1, buffer, sizeof(buffer), -1,
                                                  Clock::now(), SCRIPTED_OPS);
   require(result == 4, fmt("unlimited read completed", result));
   require(g_pollTimeouts.size() == 1 && g_pollTimeouts[0] == -1,
           "unlimited: poll() waits without limit");

   // zero: poll() is asked not to wait at all, and nothing available ends the read
   resetScript();
   g_pollAlwaysReadyWhenScriptEnds = false;
   result = detail::boundedReadFromDescriptor(-1, buffer, sizeof(buffer), 0,
                                              Clock::now(), SCRIPTED_OPS);
   require(result == 0, fmt("zero timeout, nothing available: 0", result));
   require(g_pollTimeouts.size() == 1 && g_pollTimeouts[0] == 0,
           "zero: poll() does not wait");
}

//******************************************************************************
// the receive cannot block, on a real kernel

POIVRE_TEST_CASE(TestSocketBoundedRead, testRecvCannotBlockEvenIfPollReportsReadiness) {
   HangGuard guard("testRecvCannotBlockEvenIfPollReportsReadiness");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Peer peer(pair.server(), {}, false);   // idle: a blocking recv() would hang

   char buffer[8];
   const Clock::time_point start = Clock::now();
   const int result = detail::boundedReadFromDescriptor(pair.client(),
                                                        buffer,
                                                        sizeof(buffer),
                                                        200,
                                                        start,
                                                        LYING_POLL_REAL_RECV_OPS);
   const long long elapsed = millisSince(start);

   require(result == 0, fmt("no bytes: 0", result));
   require(elapsed >= 180 && elapsed <= 200 + SLACK,
           fmt("recv() returned each time (EAGAIN) and the budget ended the read", elapsed));
}

//******************************************************************************
// 10. zero timeout

POIVRE_TEST_CASE(TestSocketBoundedRead, testZeroTimeoutTakesOnlyAvailableBytes) {
   HangGuard guard("testZeroTimeoutTakesOnlyAvailableBytes");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   const int serverFd = pair.server();
   Socket socket(pair.takeClient());

   char buffer[10];

   // everything wanted is already there
   require(sendAll(serverFd, pattern(10)), "peer sends 10 bytes");
   require(waitAvailable(socket.getFileDescriptor(), 10), "the 10 bytes have arrived");
   Clock::time_point start = Clock::now();
   int result = socket.readSocket(buffer, sizeof(buffer), 0);
   require(result == 10, fmt("all 10 available bytes", result));
   requireStringEquals(pattern(10), bytesOf(buffer, 10), "bytes");
   require(millisSince(start) < 500, "did not wait");

   // only some of it is there
   require(sendAll(serverFd, pattern(4)), "peer sends 4 bytes");
   require(waitAvailable(socket.getFileDescriptor(), 4), "the 4 bytes have arrived");
   start = Clock::now();
   result = socket.readSocket(buffer, sizeof(buffer), 0);
   require(result == 4, fmt("short count: the 4 available bytes", result));
   requireStringEquals(pattern(4), bytesOf(buffer, 4), "bytes");
   require(millisSince(start) < 500, "did not wait");

   // nothing is there
   start = Clock::now();
   result = socket.readSocket(buffer, sizeof(buffer), 0);
   require(result == -1, fmt("nothing available: -1", result));
   require(millisSince(start) < 500, "did not wait");
   require(socket.isConnected(), "still connected");
}

//******************************************************************************
// 11. negative timeout: no limit

POIVRE_TEST_CASE(TestSocketBoundedRead, testNegativeTimeoutWaitsForSegmentedData) {
   HangGuard guard("testNegativeTimeoutWaitsForSegmentedData");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Socket socket(pair.takeClient());
   Peer peer(pair.server(),
             { {0, pattern(10, 0)}, {250, pattern(10, 10)}, {500, pattern(10, 20)} },
             false);

   char buffer[30];
   const Clock::time_point start = Clock::now();
   const int result = socket.readSocket(buffer, sizeof(buffer), -1);
   const long long elapsed = millisSince(start);

   require(result == 30, fmt("waited for every segment", result));
   requireStringEquals(pattern(30), bytesOf(buffer, 30), "bytes in order");
   require(elapsed >= 400, fmt("it did wait for the slow segments", elapsed));

   // any negative value means unlimited
   Peer second(pair.server(), { {50, "xyz"} }, false);
   char small[3];
   require(socket.readSocket(small, sizeof(small), INT_MIN) == 3, "INT_MIN is also unlimited");
   requireStringEquals("xyz", bytesOf(small, 3), "bytes");
}

//******************************************************************************
// 12. primed line-input buffer

POIVRE_TEST_CASE(TestSocketBoundedRead, testPrimedBufferIsServedFirst) {
   HangGuard guard("testPrimedBufferIsServedFirst");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   PrimableSocket socket(pair.takeClient());
   const int serverFd = pair.server();
   char buffer[16];

   // primed bytes alone satisfy the read: no waiting, whatever the timeout, and
   // the peer is silent
   socket.setLineInputBuffer("hello world");
   Clock::time_point start = Clock::now();
   int result = socket.readSocket(buffer, 5, 0);
   require(result == 5, fmt("served from the primed buffer, timeout 0", result));
   requireStringEquals("hello", bytesOf(buffer, 5), "first part");
   result = socket.readSocket(buffer, 6, 5000);
   require(result == 6, fmt("the rest of the primed buffer, timeout 5000", result));
   requireStringEquals(" world", bytesOf(buffer, 6), "second part");
   require(millisSince(start) < 500, "primed bytes did not wait");

   // primed bytes first, then bytes from the socket
   socket.setLineInputBuffer("ab");
   require(sendAll(serverFd, "cdef"), "peer sends 4 bytes");
   result = socket.readSocket(buffer, 6, 3000);
   require(result == 6, fmt("primed 2 + socket 4", result));
   requireStringEquals("abcdef", bytesOf(buffer, 6), "primed bytes come first");

   // the same with timeout 0, when the socket's bytes are already there
   socket.setLineInputBuffer("ab");
   require(sendAll(serverFd, "cdef"), "peer sends 4 bytes");
   require(waitAvailable(socket.getFileDescriptor(), 4), "the 4 bytes have arrived");
   result = socket.readSocket(buffer, 6, 0);
   require(result == 6, fmt("primed 2 + available 4, timeout 0", result));
   requireStringEquals("abcdef", bytesOf(buffer, 6), "primed bytes come first");

   // the primed bytes are not enough and the peer is silent: they are returned
   // as a short count once the budget is used up
   socket.setLineInputBuffer("ab");
   start = Clock::now();
   result = socket.readSocket(buffer, 6, BUDGET);
   const long long elapsed = millisSince(start);
   require(result == 2, fmt("short count: the 2 primed bytes", result));
   requireStringEquals("ab", bytesOf(buffer, 2), "primed bytes");
   require(elapsed >= BUDGET * 9 / 10 && elapsed <= BUDGET + SLACK,
           fmt("waited for the budget for the missing bytes", elapsed));
}

//******************************************************************************
// 13. end of stream

POIVRE_TEST_CASE(TestSocketBoundedRead, testEndOfStreamEndsTheReadAtOnce) {
   HangGuard guard("testEndOfStreamEndsTheReadAtOnce");

   char buffer[10];

   // closed without a byte
   {
      TcpPair pair;
      require(pair.isOpen(), "loopback connection");
      Socket socket(pair.takeClient());
      pair.closeServer();

      const Clock::time_point start = Clock::now();
      const int result = socket.readSocket(buffer, sizeof(buffer), 5000);
      require(result == -1, fmt("EOF with nothing read: -1", result));
      require(millisSince(start) < 2500, "EOF did not wait out the budget");

      // and again
      require(socket.readSocket(buffer, sizeof(buffer), 5000) == -1, "still -1");
   }

   // closed after a partial read
   {
      TcpPair pair;
      require(pair.isOpen(), "loopback connection");
      Socket socket(pair.takeClient());
      Peer peer(pair.server(), { {0, pattern(4)} }, true);

      const Clock::time_point start = Clock::now();
      const int result = socket.readSocket(buffer, sizeof(buffer), 5000);
      require(result == 4, fmt("short count after the peer disconnected", result));
      requireStringEquals(pattern(4), bytesOf(buffer, 4), "bytes");
      require(millisSince(start) < 2500, "EOF did not wait out the budget");
   }

   // no limit: EOF still ends the read
   {
      TcpPair pair;
      require(pair.isOpen(), "loopback connection");
      Socket socket(pair.takeClient());
      Peer peer(pair.server(), { {0, pattern(4)} }, true);

      const int result = socket.readSocket(buffer, sizeof(buffer), -1);
      require(result == 4, fmt("short count after the peer disconnected, unlimited", result));
   }
}

//******************************************************************************
// 14. invalid arguments and states

POIVRE_TEST_CASE(TestSocketBoundedRead, testNullBufferAndSizes) {
   HangGuard guard("testNullBufferAndSizes");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   PrimableSocket socket(pair.takeClient());
   char buffer[8];

   require(socket.readSocket(nullptr, 4, 100) == -1, "null buffer: -1");
   require(socket.readSocket(nullptr, 0, 100) == -1, "null buffer, zero bytes: -1");
   require(socket.readSocket(nullptr, -1, 100) == -1, "null buffer, negative bytes: -1");

   // zero bytes: success with nothing read, and nothing consumed
   socket.setLineInputBuffer("abc");
   Clock::time_point start = Clock::now();
   require(socket.readSocket(buffer, 0, 5000) == 0, "zero bytes: 0");
   require(socket.readSocket(buffer, 0, -1) == 0, "zero bytes, unlimited: 0");
   require(millisSince(start) < 500, "zero bytes does not wait");
   require(socket.readSocket(buffer, 3, 0) == 3, "the primed bytes are still there");
   requireStringEquals("abc", bytesOf(buffer, 3), "primed bytes intact");

   // and with nothing primed: still 0 (the two-argument form returns -1 here)
   start = Clock::now();
   require(socket.readSocket(buffer, 0, 5000) == 0, "zero bytes, nothing primed: 0");
   require(socket.readSocket(buffer, 0, 0) == 0, "zero bytes, zero timeout: 0");
   require(millisSince(start) < 500, "zero bytes does not wait");

   // negative bytes: failure, with and without primed bytes, nothing consumed
   require(socket.readSocket(buffer, -1, 100) == -1, "negative bytes: -1");
   socket.setLineInputBuffer("abc");
   require(socket.readSocket(buffer, -5, 100) == -1, "negative bytes with primed data: -1");
   require(socket.readSocket(buffer, 3, 0) == 3, "primed bytes untouched by the failed call");
   requireStringEquals("abc", bytesOf(buffer, 3), "primed bytes intact");
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testMessageSizeModeIsRejected) {
   HangGuard guard("testMessageSizeModeIsRejected");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   PrimableSocket socket(pair.takeClient());
   const int serverFd = pair.server();
   char buffer[8];

   require(sendAll(serverFd, "abcd"), "peer sends 4 bytes");
   require(waitAvailable(socket.getFileDescriptor(), 4), "the 4 bytes have arrived");
   socket.setLineInputBuffer("xy");

   socket.setIncludeMessageSize(true);
   const Clock::time_point start = Clock::now();
   require(socket.readSocket(buffer, 4, 5000) == -1, "size mode: -1");
   require(socket.readSocket(buffer, 4, 0) == -1, "size mode, zero timeout: -1");
   require(socket.readSocket(buffer, 4, -1) == -1, "size mode, unlimited: -1");
   require(millisSince(start) < 500, "rejected at once");
   require(socket.getIncludeMessageSize(), "the mode itself is left as it was");

   // the rejected calls consumed nothing, from the primed buffer or the socket
   socket.setIncludeMessageSize(false);
   require(socket.readSocket(buffer, 6, 1000) == 6, "primed + socket bytes still all there");
   requireStringEquals("xyabcd", bytesOf(buffer, 6), "nothing was consumed");
}

POIVRE_TEST_CASE(TestSocketBoundedRead, testClosedSocketAndDescriptor) {
   HangGuard guard("testClosedSocketAndDescriptor");

   char buffer[8];

   // closed through Socket::close()
   {
      TcpPair pair;
      require(pair.isOpen(), "loopback connection");
      Socket socket(pair.takeClient());
      socket.close();
      const Clock::time_point start = Clock::now();
      require(socket.readSocket(buffer, 4, 5000) == -1, "closed socket: -1");
      require(socket.readSocket(buffer, 4, -1) == -1, "closed socket, unlimited: -1");
      require(socket.readSocket(buffer, 4, 0) == -1, "closed socket, zero timeout: -1");
      require(socket.readSocket(buffer, 0, 5000) == 0, "closed socket, zero bytes: 0 (nothing to read)");
      require(millisSince(start) < 500, "did not wait");
   }

   // closed, but with primed bytes left: those are returned as a short count
   {
      TcpPair pair;
      require(pair.isOpen(), "loopback connection");
      PrimableSocket socket(pair.takeClient());
      socket.setLineInputBuffer("ab");
      socket.close();
      require(socket.readSocket(buffer, 5, 5000) == 2, "closed socket: the primed bytes");
      requireStringEquals("ab", bytesOf(buffer, 2), "primed bytes");
   }

   // descriptor closed behind the Socket's back: poll() reports POLLNVAL
   {
      TcpPair pair;
      require(pair.isOpen(), "loopback connection");
      const int fd = pair.takeClient();
      Socket socket(fd);
      ::close(fd);
      const Clock::time_point start = Clock::now();
      require(socket.readSocket(buffer, 4, 5000) == -1, "closed descriptor: -1");
      require(socket.readSocket(buffer, 4, -1) == -1, "closed descriptor, unlimited: -1");
      require(millisSince(start) < 2500, "a closed descriptor does not wait out the budget");
   }

   // a socket that never had a descriptor
   {
      Socket socket(-1);
      require(socket.readSocket(buffer, 4, 5000) == -1, "no descriptor: -1");
   }
}

//******************************************************************************
// the socket's own state is left alone

POIVRE_TEST_CASE(TestSocketBoundedRead, testSocketSettingsAreNotChanged) {
   HangGuard guard("testSocketSettingsAreNotChanged");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Socket socket(pair.takeClient());
   Peer peer(pair.server(), { {0, "abc"} }, false);

   require(socket.setReceiveTimeout(7), "set a persistent receive timeout");
   const int fd = socket.getFileDescriptor();
   const int flagsBefore = ::fcntl(fd, F_GETFL, 0);
   require(flagsBefore >= 0 && (flagsBefore & O_NONBLOCK) == 0, "descriptor starts in blocking mode");

   char buffer[8];
   require(socket.readSocket(buffer, sizeof(buffer), BUDGET) == 3, "short count after the timeout");
   require(socket.readSocket(buffer, sizeof(buffer), 0) == -1, "nothing available");

   require(socket.getFileDescriptor() == fd, "same descriptor");
   require(socket.isConnected(), "still connected");
   const int flagsAfter = ::fcntl(fd, F_GETFL, 0);
   require(flagsAfter == flagsBefore, "file status flags unchanged: still blocking");

   struct timeval tv;
   socklen_t len = sizeof(tv);
   ::memset(&tv, 0, sizeof(tv));
   require(::getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, &len) == 0, "read SO_RCVTIMEO");
   require(tv.tv_sec == 7 && tv.tv_usec == 0, "SO_RCVTIMEO is still the 7 seconds that were set");
}

//******************************************************************************
// a very large budget

POIVRE_TEST_CASE(TestSocketBoundedRead, testHugeTimeoutDoesNotOverflow) {
   HangGuard guard("testHugeTimeoutDoesNotOverflow");

   TcpPair pair;
   require(pair.isOpen(), "loopback connection");

   Socket socket(pair.takeClient());
   Peer peer(pair.server(), { {50, "abcd"}, {100, "efgh"} }, false);

   char buffer[8];
   const Clock::time_point start = Clock::now();
   const int result = socket.readSocket(buffer, sizeof(buffer), INT_MAX);
   require(result == 8, fmt("read completed with a budget of INT_MAX ms", result));
   requireStringEquals("abcdefgh", bytesOf(buffer, 8), "bytes");
   require(millisSince(start) < 3000, "and did not wait for the budget");
}

//******************************************************************************
// 15. the two-argument readSocket() is unchanged

POIVRE_TEST_CASE(TestSocketBoundedRead, testTwoArgumentReadSocketStillWorks) {
   HangGuard guard("testTwoArgumentReadSocketStillWorks");

   char buffer[30];

   // segmented delivery
   {
      TcpPair pair;
      require(pair.isOpen(), "loopback connection");
      Socket socket(pair.takeClient());
      Peer peer(pair.server(),
                { {0, pattern(10, 0)}, {60, pattern(10, 10)}, {120, pattern(10, 20)} },
                false);
      const int result = socket.readSocket(buffer, 30);
      require(result == 30, fmt("two-argument read of segmented data", result));
      requireStringEquals(pattern(30), bytesOf(buffer, 30), "bytes in order");
   }

   // end of stream after a partial read: the short count, as before
   {
      TcpPair pair;
      require(pair.isOpen(), "loopback connection");
      Socket socket(pair.takeClient());
      Peer peer(pair.server(), { {0, pattern(4)} }, true);
      const int result = socket.readSocket(buffer, 10);
      require(result == 4, fmt("two-argument short count at EOF", result));
   }

   // the unlimited three-argument form returns what the two-argument form does
   {
      TcpPair pair;
      require(pair.isOpen(), "loopback connection");
      Socket socket(pair.takeClient());
      Peer peer(pair.server(),
                { {0, pattern(10, 0)}, {40, pattern(10, 10)}, {80, pattern(10, 20)} },
                false);
      const int result = socket.readSocket(buffer, 30, -1);
      require(result == 30, fmt("three-argument unlimited read", result));
      requireStringEquals(pattern(30), bytesOf(buffer, 30), "same bytes");
   }

   // null buffer, as before
   {
      TcpPair pair;
      require(pair.isOpen(), "loopback connection");
      Socket socket(pair.takeClient());
      require(socket.readSocket(nullptr, 4) == -1, "two-argument null buffer: -1");
   }
}
