// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

// Tests that a blocking send() that a signal interrupts does not make Socket::write() abandon the
// rest of the transmission (defect F-02).
//
// send() on a blocked socket returns the bytes it has copied as a positive count when a signal
// interrupts it after some progress, and fails with EINTR only when that call transferred nothing.
// sendPayload() used to treat the EINTR as the end of the transmission, leaving the receiver with a
// prefix of the message. It now repeats the interrupted call from the same position; the same holds
// for the two-byte size prefix of sendPayloadSize(). Every other negative result (EAGAIN, EPIPE,
// ECONNRESET, ...) is still terminal and a zero result is still not handled (F-03, not changed here).
//
// How an interrupted send() is made observable: Socket sends through chaudiere::detail::sendIoOps()
// (src/internal/SocketIo.h), which is the real ::send() unless a test replaces it.
//   - Scripted tests replace it with a function that performs REAL sends on a real loopback
//     connection, but shortens a call, or fails it with -1/errno without sending, as a script says.
//     It records every call: bytes requested, offset of the buffer within the payload, result, errno
//     and flags. The tests compare that whole sequence with the expected one and then read the bytes
//     from the peer, so a retry that used the wrong offset or length cannot pass: it would send the
//     wrong bytes and the peer's copy would differ from the payload.
//   - The real-signal test replaces it with a wrapper that performs the real send() and counts what
//     the kernel returned (EINTR results, short counts, bytes). A signal that was merely delivered
//     proves nothing: the test passes only if send() really returned -1/EINTR inside Socket::write(),
//     and reports INCONCLUSIVE when it did not.
// The scripted function only acts on a thread that has a script (thread_local); any other thread's
// sends pass straight through. Overrides are restored by guards; every check runs in a forked child.
//
// Real signals: SIGUSR1, handler installed without SA_RESTART, sent by pthread_kill to the one thread
// that is blocked in send(); a loopback connection with small send/receive buffers, a stalled receiver
// and a large payload (the conditions of the T2c-1 investigation).
//
// Results: PASS, FAIL, or INCONCLUSIVE when the environment cannot establish the condition (the send
// did not block, the handler did not run in the writing thread, no EINTR was returned). INCONCLUSIVE is
// printed as such and never counted as evidence that the retry works.
//
// Isolation and limits: every check is a forked child (ChildProcess.h) with a hard time limit. Loopback
// only, ports chosen by the kernel, no root.

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
#include <sys/time.h>
#include <unistd.h>

#include "TestSocketSendInterrupt.h"
#include "ChildProcess.h"
#include "Socket.h"
#include "internal/SocketIo.h"

using namespace chaudiere;
using namespace chaudiere_test;

//******************************************************************************

TestSocketSendInterrupt::TestSocketSendInterrupt() :
   poivre::TestSuite("TestSocketSendInterrupt") {
}

//******************************************************************************

namespace {

const int CHILD_INCONCLUSIVE = CHILD_SKIPPED;
const int TIME_LIMIT_MILLIS = 40000;
const long long SAFETY_MILLIS = 15000;       // the real-signal check's own limit for the transmission
const long long BLOCK_CHECK_MILLIS = 250;    // the writer must still be inside send() this long after it started

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

// the payload: the byte at position i is i % 251, so any misplaced, repeated or missing byte shows
std::string patternBytes(size_t n) {
   std::string s(n, '\0');
   for (size_t i = 0; i < n; ++i) {
      s[i] = static_cast<char>(i % 251);
   }
   return s;
}

// how many bytes from the start of data are the same as the pattern (data's first byte is pattern position 0)
size_t patternPrefixLength(const std::string& data, size_t from = 0) {
   size_t i = 0;
   while (i < data.size() && static_cast<unsigned char>(data[i]) == static_cast<unsigned char>((from + i) % 251)) {
      ++i;
   }
   return i;
}

sockaddr_in loopbackAddress(int port) {
   sockaddr_in addr;
   ::memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   addr.sin_port = htons(static_cast<unsigned short>(port));
   return addr;
}

int portOf(int fd) {
   sockaddr_in addr;
   socklen_t length = sizeof(addr);
   return (::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &length) == 0) ? ntohs(addr.sin_port) : -1;
}

// a blocking loopback listener on a kernel-chosen port; rcvbuf > 0 sets SO_RCVBUF before listening, so
// that the accepted socket has it too
int listenLoopback(int rcvbuf, int& port) {
   const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0) {
      return -1;
   }
   const int on = 1;
   ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
   if (rcvbuf > 0) {
      ::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
   }
   sockaddr_in addr = loopbackAddress(0);
   if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 ||
       ::listen(fd, 4) != 0) {
      ::close(fd);
      return -1;
   }
   port = portOf(fd);
   return fd;
}

// waits for fd to become readable; true if it did within millis
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

// reads up to count bytes (stops at end of stream, or when nothing more arrives within idleMillis)
std::string readUpTo(int fd, size_t count, long long idleMillis) {
   std::string out;
   char buffer[65536];
   while (out.size() < count) {
      if (!readableWithin(fd, idleMillis)) {
         break;
      }
      const size_t want = (count - out.size() < sizeof(buffer)) ? (count - out.size()) : sizeof(buffer);
      const ssize_t n = ::recv(fd, buffer, want, 0);
      if (n <= 0) {
         break;
      }
      out.append(buffer, static_cast<size_t>(n));
   }
   return out;
}

//******************************************************************************
// the scripted send(): real sends, shortened or failed as a script says, every call recorded

enum StepKind {
   STEP_REAL,       // the real send() of everything asked for
   STEP_SHORT,      // the real send() of at most `value` bytes
   STEP_FAIL        // -1 with errno = value, nothing is sent
};

struct Step {
   StepKind kind;
   int value;
};

struct Call {
   long requested;
   long offset;      // of the buffer within the payload; -1 for any other buffer (the size prefix)
   long returned;
   int error;        // errno of a failed call, else 0
   int flags;
};

struct Script {
   std::vector<Step> steps;
   size_t next = 0;
   std::vector<Call> calls;
   const char* payload = nullptr;
   size_t payloadSize = 0;
};

thread_local Script* t_script = nullptr;

ssize_t scriptedSend(int fd, const void* buffer, size_t length, int flags) {
   Script* script = t_script;
   if (script == nullptr) {
      return ::send(fd, buffer, length, flags);
   }

   Step step;
   step.kind = STEP_REAL;
   step.value = 0;
   if (script->next < script->steps.size()) {
      step = script->steps[script->next++];
   }

   Call call;
   call.requested = static_cast<long>(length);
   const char* at = static_cast<const char*>(buffer);
   call.offset = ((at >= script->payload) && (at <= script->payload + script->payloadSize)) ?
                 static_cast<long>(at - script->payload) : -1;
   call.flags = flags;
   call.error = 0;

   ssize_t result;
   if (step.kind == STEP_FAIL) {
      result = -1;
      call.error = step.value;
   } else {
      const size_t n = (step.kind == STEP_SHORT && static_cast<size_t>(step.value) < length) ?
                       static_cast<size_t>(step.value) : length;
      result = ::send(fd, buffer, n, flags);
      if (result < 0) {
         call.error = errno;
      }
   }
   call.returned = static_cast<long>(result);
   script->calls.push_back(call);
   errno = call.error;
   return result;
}

const detail::SendIoOps SCRIPTED_OPS = { scriptedSend };

// replaces the send for the life of the guard, and puts back whatever was there
class SendOverride {
public:
   explicit SendOverride(const detail::SendIoOps* ops) : m_previous(detail::setSendIoOps(ops)) {}
   ~SendOverride() { detail::setSendIoOps(m_previous); }
   SendOverride(const SendOverride&) = delete;
   SendOverride& operator=(const SendOverride&) = delete;
private:
   const detail::SendIoOps* m_previous;
};

class ScriptScope {
public:
   explicit ScriptScope(Script* script) : m_previous(t_script) { t_script = script; }
   ~ScriptScope() { t_script = m_previous; }
   ScriptScope(const ScriptScope&) = delete;
   ScriptScope& operator=(const ScriptScope&) = delete;
private:
   Script* m_previous;
};

struct Expected {
   long requested;
   long offset;
   long returned;
   int error;
};

std::string describeCalls(const std::vector<Call>& calls) {
   std::string s;
   for (const Call& c : calls) {
      char b[96];
      if (c.returned < 0) {
         snprintf(b, sizeof(b), " [req %ld @%ld -> -1 errno %d]", c.requested, c.offset, c.error);
      } else {
         snprintf(b, sizeof(b), " [req %ld @%ld -> %ld]", c.requested, c.offset, c.returned);
      }
      s += b;
   }
   return s;
}

bool sameCalls(const std::vector<Call>& calls, const std::vector<Expected>& expected) {
   if (calls.size() != expected.size()) {
      return false;
   }
   for (size_t i = 0; i < calls.size(); ++i) {
      if (calls[i].requested != expected[i].requested || calls[i].offset != expected[i].offset ||
          calls[i].returned != expected[i].returned || calls[i].error != expected[i].error) {
         return false;
      }
   }
   return true;
}

// One scripted write of PAYLOAD bytes over a real loopback connection. Checks: the result of
// Socket::write(), errno right after a failure, the exact sequence of send() calls, that every call kept
// the SIGPIPE protection flag, and the bytes the peer received (which must be the payload's first
// `deliveredBytes` bytes, in order, and nothing more).
struct ScriptedRun {
   std::vector<Step> steps;
   std::vector<Expected> expectedCalls;
   bool sizePrefix = false;
   bool expectOk = true;
   int expectedErrno = 0;
   size_t deliveredPayloadBytes = 0;
   bool checkErrno = true;          // errno means something only after a failed call
   bool skipDelivery = false;       // the peer's bytes are not judged (a case whose point is only that write() failed)
};

const size_t PAYLOAD = 4096;

int runScripted(FILE* note, const ScriptedRun& run) {
   const int fdBefore = lowestFreeDescriptor();
   int port = 0;
   const int listenFd = listenLoopback(0, port);
   if (listenFd < 0) {
      fprintf(note, "could not create a listener");
      return CHILD_SETUP_FAILED;
   }

   int result = CHILD_OK;
   {
      Socket client("127.0.0.1", port);
      const int serverFd = ::accept(listenFd, nullptr, nullptr);
      if (serverFd < 0) {
         fprintf(note, "could not accept");
         ::close(listenFd);
         return CHILD_SETUP_FAILED;
      }
      client.setIncludeMessageSize(run.sizePrefix);

      const std::string payload = patternBytes(PAYLOAD);
      Script script;
      script.steps = run.steps;
      script.payload = payload.data();
      script.payloadSize = payload.size();

      bool ok;
      int errnoAfter;
      {
         SendOverride override(&SCRIPTED_OPS);
         ScriptScope scope(&script);
         ok = client.write(payload.data(), payload.size());
         errnoAfter = errno;
      }

      const size_t prefixBytes = run.sizePrefix ? 2 : 0;
      const std::string received = readUpTo(serverFd, prefixBytes + run.deliveredPayloadBytes + 64, 200);
      ::close(serverFd);


      // the verdicts, most specific first
      if (ok != run.expectOk) {
         fprintf(note, "Socket::write returned %s, expected %s; calls:%s", ok ? "true" : "false", run.expectOk ? "true" : "false", describeCalls(script.calls).c_str());
         result = CHILD_CHECK_FAILED;
      } else if (!run.expectOk && run.checkErrno && errnoAfter != run.expectedErrno) {
         fprintf(note, "errno after the failure was %d (%s), expected %d (%s); calls:%s", errnoAfter, strerror(errnoAfter),
                 run.expectedErrno, strerror(run.expectedErrno), describeCalls(script.calls).c_str());
         result = CHILD_CHECK_FAILED;
      } else if (!sameCalls(script.calls, run.expectedCalls)) {
         fprintf(note, "the sequence of send() calls is not the expected one; got:%s", describeCalls(script.calls).c_str());
         result = CHILD_CHECK_FAILED;
      } else {
         bool flagsKept = true;
#ifdef MSG_NOSIGNAL
         for (const Call& c : script.calls) {
            if ((c.flags & MSG_NOSIGNAL) == 0) {
               flagsKept = false;
            }
         }
#endif
         std::string deliveredPayload = received;
         bool prefixOk = true;
         if (run.sizePrefix) {
            prefixOk = (received.size() >= 2) &&
                       (static_cast<unsigned char>(received[0]) == ((PAYLOAD >> 8) & 0xff)) &&
                       (static_cast<unsigned char>(received[1]) == (PAYLOAD & 0xff));
            deliveredPayload = (received.size() >= 2) ? received.substr(2) : std::string();
         }
         if (run.skipDelivery) {
            fprintf(note, "%zu send() call(s):%s", script.calls.size(), describeCalls(script.calls).c_str());
         } else if (!flagsKept) {
            fprintf(note, "a send() call lost MSG_NOSIGNAL (SIGPIPE protection); calls:%s", describeCalls(script.calls).c_str());
            result = CHILD_CHECK_FAILED;
         } else if (!prefixOk) {
            fprintf(note, "the two-byte size prefix received is not the payload size in network order; calls:%s", describeCalls(script.calls).c_str());
            result = CHILD_CHECK_FAILED;
         } else if (deliveredPayload.size() != run.deliveredPayloadBytes ||
                    patternPrefixLength(deliveredPayload) != deliveredPayload.size()) {
            fprintf(note, "the peer received %zu payload bytes (%zu of them as in the payload), expected exactly the first %zu in order; calls:%s",
                    deliveredPayload.size(), patternPrefixLength(deliveredPayload), run.deliveredPayloadBytes, describeCalls(script.calls).c_str());
            result = CHILD_CHECK_FAILED;
         } else {
            fprintf(note, "%zu send() call(s):%s; the peer received %zu payload bytes, in order",
                    script.calls.size(), describeCalls(script.calls).c_str(), deliveredPayload.size());
         }
      }
   }
   ::close(listenFd);

   if (result == CHILD_OK && lowestFreeDescriptor() != fdBefore) {
      fprintf(note, " [a descriptor leaked]");
      return CHILD_CHECK_FAILED;
   }
   return result;
}

//******************************************************************************
// the scripted checks (each runs in a child)

Step step(StepKind kind, int value = 0) {
   Step s;
   s.kind = kind;
   s.value = value;
   return s;
}

const long P = static_cast<long>(PAYLOAD);

// R1: an ordinary write is one send() of everything
int checkOrdinary(FILE* note) {
   ScriptedRun r;
   r.expectedCalls = { { P, 0, P, 0 } };
   r.deliveredPayloadBytes = PAYLOAD;
   return runScripted(note, r);
}

// R2: positive short counts, no error: every call continues at the right offset with the right length
int checkShortSends(FILE* note) {
   ScriptedRun r;
   r.steps = { step(STEP_SHORT, 1000), step(STEP_SHORT, 1000), step(STEP_SHORT, 1000), step(STEP_SHORT, 1000) };
   r.expectedCalls = { { P, 0, 1000, 0 }, { P - 1000, 1000, 1000, 0 }, { P - 2000, 2000, 1000, 0 },
                       { P - 3000, 3000, 1000, 0 }, { P - 4000, 4000, P - 4000, 0 } };
   r.deliveredPayloadBytes = PAYLOAD;
   return runScripted(note, r);
}

// R3: EINTR before any progress: the same call again, from the start
int checkInterruptedBeforeProgress(FILE* note) {
   ScriptedRun r;
   r.steps = { step(STEP_FAIL, EINTR) };
   r.expectedCalls = { { P, 0, -1, EINTR }, { P, 0, P, 0 } };
   r.deliveredPayloadBytes = PAYLOAD;
   return runScripted(note, r);
}

// R4: the F-02 case: a short count, then EINTR, then the rest from the offset the short count reached
int checkInterruptedAfterPartialProgress(FILE* note) {
   ScriptedRun r;
   r.steps = { step(STEP_SHORT, 1500), step(STEP_FAIL, EINTR) };
   r.expectedCalls = { { P, 0, 1500, 0 }, { P - 1500, 1500, -1, EINTR }, { P - 1500, 1500, P - 1500, 0 } };
   r.deliveredPayloadBytes = PAYLOAD;
   return runScripted(note, r);
}

// R5: several interruptions in a row, between short counts
int checkRepeatedInterruptions(FILE* note) {
   ScriptedRun r;
   r.steps = { step(STEP_FAIL, EINTR), step(STEP_FAIL, EINTR), step(STEP_FAIL, EINTR), step(STEP_SHORT, 700),
               step(STEP_FAIL, EINTR), step(STEP_FAIL, EINTR), step(STEP_SHORT, 300), step(STEP_FAIL, EINTR) };
   r.expectedCalls = { { P, 0, -1, EINTR }, { P, 0, -1, EINTR }, { P, 0, -1, EINTR }, { P, 0, 700, 0 },
                       { P - 700, 700, -1, EINTR }, { P - 700, 700, -1, EINTR }, { P - 700, 700, 300, 0 },
                       { P - 1000, 1000, -1, EINTR }, { P - 1000, 1000, P - 1000, 0 } };
   r.deliveredPayloadBytes = PAYLOAD;
   return runScripted(note, r);
}

// R6: every other error stays terminal, after earlier progress; the REAL step that follows in the
// script must never be reached (no retry), and errno is the error's
int checkOtherErrorsStayTerminal(FILE* note) {
   const int errors[] = { EAGAIN, EWOULDBLOCK, EPIPE, ECONNRESET, EBADF, EIO };
   for (int error : errors) {
      ScriptedRun r;
      r.steps = { step(STEP_SHORT, 500), step(STEP_FAIL, error), step(STEP_REAL) };
      r.expectedCalls = { { P, 0, 500, 0 }, { P - 500, 500, -1, error } };
      r.expectOk = false;
      r.expectedErrno = error;
      r.deliveredPayloadBytes = 500;
      const int rc = runScripted(note, r);
      if (rc != CHILD_OK) {
         fprintf(note, " [error %d (%s)]", error, strerror(error));
         return rc;
      }
      fprintf(note, "; ");
   }
   // an interruption is retried, the terminal error after it is not
   ScriptedRun r;
   r.steps = { step(STEP_FAIL, EINTR), step(STEP_FAIL, EPIPE), step(STEP_REAL) };
   r.expectedCalls = { { P, 0, -1, EINTR }, { P, 0, -1, EPIPE } };
   r.expectOk = false;
   r.expectedErrno = EPIPE;
   r.deliveredPayloadBytes = 0;
   return runScripted(note, r);
}

// R7: the two-byte size prefix
int checkInterruptedSizePrefix(FILE* note) {
   {
      ScriptedRun r;
      r.sizePrefix = true;
      r.steps = { step(STEP_FAIL, EINTR) };
      r.expectedCalls = { { 2, -1, -1, EINTR }, { 2, -1, 2, 0 }, { P, 0, P, 0 } };
      r.deliveredPayloadBytes = PAYLOAD;
      const int rc = runScripted(note, r);
      if (rc != CHILD_OK) {
         return rc;
      }
   }
   fprintf(note, "; ");
   {
      ScriptedRun r;
      r.sizePrefix = true;
      r.steps = { step(STEP_FAIL, EINTR), step(STEP_FAIL, EINTR) };
      r.expectedCalls = { { 2, -1, -1, EINTR }, { 2, -1, -1, EINTR }, { 2, -1, 2, 0 }, { P, 0, P, 0 } };
      r.deliveredPayloadBytes = PAYLOAD;
      const int rc = runScripted(note, r);
      if (rc != CHILD_OK) {
         return rc;
      }
   }
   fprintf(note, "; ");
   // an interrupted prefix, then an interrupted payload after partial progress
   ScriptedRun r;
   r.sizePrefix = true;
   r.steps = { step(STEP_FAIL, EINTR), step(STEP_REAL), step(STEP_SHORT, 1000), step(STEP_FAIL, EINTR) };
   r.expectedCalls = { { 2, -1, -1, EINTR }, { 2, -1, 2, 0 }, { P, 0, 1000, 0 }, { P - 1000, 1000, -1, EINTR },
                       { P - 1000, 1000, P - 1000, 0 } };
   r.deliveredPayloadBytes = PAYLOAD;
   return runScripted(note, r);
}

// the prefix is still sent by one call that must transfer both bytes: a short count (one byte) is
// still a failure, and no payload byte is sent. Existing behaviour, deliberately unchanged here.
int checkShortSizePrefixStillFails(FILE* note) {
   ScriptedRun r;
   r.sizePrefix = true;
   r.steps = { step(STEP_SHORT, 1) };
   r.expectedCalls = { { 2, -1, 1, 0 } };
   r.expectOk = false;
   r.checkErrno = false;
   r.skipDelivery = true;
   return runScripted(note, r);
}

// R9: the peer is gone (real sends, no script): a terminal error, not a retry and not SIGPIPE
int checkPeerDisconnect(FILE* note) {
   signal(SIGPIPE, SIG_DFL);      // the child's own disposition; if SIGPIPE were raised the child would die of it
   const int fdBefore = lowestFreeDescriptor();
   int port = 0;
   const int listenFd = listenLoopback(0, port);
   if (listenFd < 0) {
      fprintf(note, "could not create a listener");
      return CHILD_SETUP_FAILED;
   }
   bool failed = false;
   int error = 0;
   int attempts = 0;
   long long failedAfter = 0;
   {
      Socket client("127.0.0.1", port);
      const int serverFd = ::accept(listenFd, nullptr, nullptr);
      if (serverFd < 0) {
         fprintf(note, "could not accept");
         ::close(listenFd);
         return CHILD_SETUP_FAILED;
      }
      ::close(serverFd);                         // the peer disconnects
      const std::string chunk = patternBytes(65536);
      const long long start = nowMs();
      for (attempts = 1; attempts <= 60 && !failed; ++attempts) {
         if (!client.write(chunk.data(), chunk.size())) {
            failed = true;
            error = errno;
            failedAfter = nowMs() - start;
         } else {
            sleepMs(25);
         }
      }
      --attempts;
   }
   ::close(listenFd);

   if (!failed) {
      fprintf(note, "60 writes to a disconnected peer all succeeded");
      return CHILD_CHECK_FAILED;
   }
   if (error != EPIPE && error != ECONNRESET) {
      fprintf(note, "the write failed with errno %d (%s), expected EPIPE or ECONNRESET", error, strerror(error));
      return CHILD_CHECK_FAILED;
   }
   if (lowestFreeDescriptor() != fdBefore) {
      fprintf(note, "a descriptor leaked");
      return CHILD_CHECK_FAILED;
   }
   fprintf(note, "the write to a disconnected peer failed at attempt %d (%lld ms) with errno %d (%s); the process survived with SIGPIPE at its default disposition",
           attempts, failedAfter, error, strerror(error));
   return CHILD_OK;
}

//******************************************************************************
// R8: a real signal, a real blocked send(); the kernel's results are counted

std::atomic<long> g_enter(0);            // entries into send()
std::atomic<long> g_leave(0);            // returns from send()
std::atomic<long> g_calls(0);
std::atomic<long> g_eintr(0);            // -1/EINTR returned by the kernel
std::atomic<long> g_short(0);            // positive results smaller than the request
std::atomic<long> g_positiveBytes(0);
std::atomic<long> g_otherErrors(0);

ssize_t countingSend(int fd, const void* buffer, size_t length, int flags) {
   ++g_enter;
   const ssize_t result = ::send(fd, buffer, length, flags);
   const int savedErrno = errno;
   ++g_calls;
   if (result > 0) {
      g_positiveBytes += result;
      if (static_cast<size_t>(result) < length) {
         ++g_short;
      }
   } else if (result < 0) {
      if (savedErrno == EINTR) {
         ++g_eintr;
      } else {
         ++g_otherErrors;
      }
   }
   ++g_leave;
   errno = savedErrno;
   return result;
}

const detail::SendIoOps COUNTING_OPS = { countingSend };

// The handler runs in the writing thread and the main thread reads what it left: lock-free atomics.
static_assert(std::atomic<int>::is_always_lock_free, "the handler counter must be lock-free");
static_assert(std::atomic<pthread_t>::is_always_lock_free, "the handler thread id must be lock-free");

std::atomic<int> g_signals(0);
std::atomic<pthread_t> g_signalThread;

void onSignal(int) {
   g_signalThread.store(pthread_self());
   g_signals.fetch_add(1);
}

int checkRealInterruptedSend(FILE* note) {
   const size_t BIG = 2 * 1024 * 1024;
   const int fdBefore = lowestFreeDescriptor();
   g_enter = 0; g_leave = 0; g_calls = 0; g_eintr = 0; g_short = 0; g_positiveBytes = 0; g_otherErrors = 0; g_signals = 0;

   int port = 0;
   int listenFd = listenLoopback(4096, port);      // small receive buffer, inherited by the accepted socket
   if (listenFd < 0) {
      fprintf(note, "could not create a listener");
      return CHILD_SETUP_FAILED;
   }

   struct sigaction action;
   struct sigaction previousAction;
   memset(&action, 0, sizeof(action));
   action.sa_handler = onSignal;
   sigemptyset(&action.sa_mask);
   action.sa_flags = 0;                            // deliberately no SA_RESTART
   sigaction(SIGUSR1, &action, &previousAction);

   sigset_t usr1;
   sigset_t previousMask;
   sigemptyset(&usr1);
   sigaddset(&usr1, SIGUSR1);
   pthread_sigmask(SIG_BLOCK, &usr1, &previousMask);       // only the writing thread may run the handler

   SendOverride override(&COUNTING_OPS);

   std::atomic<bool> release(false);
   std::atomic<bool> peerDone(false);
   std::atomic<long> peerTotal(0);
   std::atomic<long> peerPrefix(0);
   std::atomic<bool> peerBroken(false);

   Socket client("127.0.0.1", port);
   client.setSendBufferSize(4096);

   std::thread peer([&]() {
      const int fd = ::accept(listenFd, nullptr, nullptr);
      auto take = [&](size_t count) {
         const std::string got = readUpTo(fd, count, 2000);
         for (size_t i = 0; i < got.size(); ++i) {
            const long position = peerTotal.load() + static_cast<long>(i);
            if (!peerBroken && static_cast<unsigned char>(got[i]) == static_cast<unsigned char>(position % 251)) {
               peerPrefix = position + 1;
            } else {
               peerBroken = true;
            }
         }
         peerTotal += static_cast<long>(got.size());
         return got.size();
      };
      take(16384);                                  // some bytes at once, then the receiver stalls
      const long long stallStart = nowMs();
      while (!release && nowMs() - stallStart < 1500) {
         sleepMs(5);
      }
      for (;;) {                                    // then drains to the end of the stream
         if (take(65536) == 0) {
            break;
         }
      }
      if (fd >= 0) {
         ::close(fd);
      }
      peerDone = true;
   });

   const std::string payload = patternBytes(BIG);
   std::atomic<bool> started(false);
   std::atomic<bool> finished(false);
   std::atomic<long long> startedAt(0);
   bool writeOk = false;
   std::thread writer([&]() {
      sigset_t unblock;
      sigemptyset(&unblock);
      sigaddset(&unblock, SIGUSR1);
      pthread_sigmask(SIG_UNBLOCK, &unblock, nullptr);
      startedAt = nowMs();
      started = true;
      writeOk = client.write(payload.data(), payload.size());
      finished = true;
   });

   // ends everything that was started (the writer is let finish by releasing the receiver)
   auto wrapUp = [&]() {
      release = true;
      const long long limit = nowMs() + SAFETY_MILLIS;
      while (!finished && nowMs() < limit) {
         sleepMs(5);
      }
      if (!finished) {
         fflush(note);
         ::_exit(CHILD_CHECK_FAILED);               // a stuck writer cannot be joined; the child ends here
      }
      writer.join();
      client.close();
      const long long peerLimit = nowMs() + 10000;
      while (!peerDone && nowMs() < peerLimit) {
         sleepMs(5);
      }
      if (peerDone) {
         peer.join();
      } else {
         fflush(note);
         ::_exit(CHILD_CHECK_FAILED);
      }
      ::close(listenFd);
      sigaction(SIGUSR1, &previousAction, nullptr);
      pthread_sigmask(SIG_SETMASK, &previousMask, nullptr);
   };

   while (!started) {
      sleepMs(1);
   }
   const long long t0 = startedAt;

   // the writer must be inside send() now: not finished, and a send() entered and not yet returned
   while (nowMs() - t0 < BLOCK_CHECK_MILLIS && !finished) {
      sleepMs(5);
   }
   const bool blocked = !finished && (g_enter.load() > g_leave.load());
   if (!blocked) {
      wrapUp();
      fprintf(note, "the writer was not blocked inside send() %lld ms after it started", BLOCK_CHECK_MILLIS);
      return CHILD_INCONCLUSIVE;
   }

   // signals: the first ends the blocked call with a short count (progress is not an error); a later call
   // that made no progress is the one that fails with EINTR, hence several, 60 ms apart
   int onWriter = 0;
   const long long at[] = { 0, 250, 310, 370, 430, 490, 550 };
   for (long long offset : at) {
      while (nowMs() - (t0 + BLOCK_CHECK_MILLIS) < offset && !finished) {
         sleepMs(2);
      }
      if (finished) {
         break;
      }
      const int before = g_signals.load();
      ::pthread_kill(writer.native_handle(), SIGUSR1);
      const long long limit = nowMs() + 300;
      while (g_signals.load() == before && nowMs() < limit) {
         sleepMs(2);
      }
      const pthread_t handlerThread = g_signalThread.load();
      if (g_signals.load() != before && pthread_equal(handlerThread, writer.native_handle())) {
         ++onWriter;
      }
   }

   wrapUp();

   const long eintr = g_eintr.load();
   const long shortCounts = g_short.load();
   char facts[300];
   snprintf(facts, sizeof(facts), "%ld send() calls, %ld returned EINTR, %ld a short count, %ld other errors, %ld bytes accepted; %d signal(s) handled, %d on the writing thread",
            g_calls.load(), eintr, shortCounts, g_otherErrors.load(), g_positiveBytes.load(), g_signals.load(), onWriter);

   if (onWriter < 1) {
      fprintf(note, "no signal handler ran in the writing thread; %s", facts);
      return CHILD_INCONCLUSIVE;
   }
   if (eintr < 1) {
      if (!writeOk) {
         fprintf(note, "Socket::write failed although send() was never interrupted; %s", facts);
         return CHILD_CHECK_FAILED;
      }
      fprintf(note, "no send() returned EINTR, so the retry was not exercised; %s", facts);
      return CHILD_INCONCLUSIVE;
   }
   if (!writeOk) {
      fprintf(note, "send() returned EINTR %ld time(s) and Socket::write gave up (peer got %ld of %zu bytes); %s", eintr, peerTotal.load(), BIG, facts);
      return CHILD_CHECK_FAILED;
   }
   if (peerTotal.load() != static_cast<long>(BIG) || peerPrefix.load() != static_cast<long>(BIG) || peerBroken) {
      fprintf(note, "the peer received %ld of %zu bytes, %ld of them in order; %s", peerTotal.load(), BIG, peerPrefix.load(), facts);
      return CHILD_CHECK_FAILED;
   }
   if (g_positiveBytes.load() != static_cast<long>(BIG)) {
      fprintf(note, "the kernel accepted %ld bytes for a %zu byte payload (a byte was sent twice or not at all); %s", g_positiveBytes.load(), BIG, facts);
      return CHILD_CHECK_FAILED;
   }
   if (lowestFreeDescriptor() != fdBefore) {
      fprintf(note, "a descriptor leaked; %s", facts);
      return CHILD_CHECK_FAILED;
   }
   fprintf(note, "interrupted inside Socket::write (%ld EINTR), completed: all %zu bytes arrived in order, none twice, no descriptor leaked; %s",
           eintr, BIG, facts);
   return CHILD_OK;
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
// R1 - R7, R9: scripted and real-peer checks

POIVRE_TEST_CASE(TestSocketSendInterrupt, testOrdinaryWriteTransmitsEverything) {
   const ChildResult r = runInChild(checkOrdinary, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("ordinary write", r);
}

POIVRE_TEST_CASE(TestSocketSendInterrupt, testPositiveShortSendsAreAccountedFor) {
   const ChildResult r = runInChild(checkShortSends, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("positive short sends", r);
}

POIVRE_TEST_CASE(TestSocketSendInterrupt, testInterruptedSendBeforeProgressIsRetried) {
   const ChildResult r = runInChild(checkInterruptedBeforeProgress, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("EINTR before progress", r);
}

POIVRE_TEST_CASE(TestSocketSendInterrupt, testInterruptedSendAfterPartialProgressContinuesAtTheRightOffset) {
   const ChildResult r = runInChild(checkInterruptedAfterPartialProgress, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("EINTR after partial progress", r);
}

POIVRE_TEST_CASE(TestSocketSendInterrupt, testRepeatedInterruptionsAreRetried) {
   const ChildResult r = runInChild(checkRepeatedInterruptions, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("repeated EINTR", r);
}

POIVRE_TEST_CASE(TestSocketSendInterrupt, testOtherErrorsRemainTerminal) {
   const ChildResult r = runInChild(checkOtherErrorsStayTerminal, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("terminal errors", r);
}

POIVRE_TEST_CASE(TestSocketSendInterrupt, testInterruptedSizePrefixIsRetried) {
   const ChildResult r = runInChild(checkInterruptedSizePrefix, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("EINTR in the size prefix", r);
}

POIVRE_TEST_CASE(TestSocketSendInterrupt, testShortSizePrefixStillFails) {
   const ChildResult r = runInChild(checkShortSizePrefixStillFails, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("short size prefix (unchanged behaviour)", r);
}

POIVRE_TEST_CASE(TestSocketSendInterrupt, testPeerDisconnectIsTerminalAndRaisesNoSigpipe) {
   const ChildResult r = runInChild(checkPeerDisconnect, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("peer disconnect", r);
}

//******************************************************************************
// R8: a real signal interrupting a real blocked send()

POIVRE_TEST_CASE(TestSocketSendInterrupt, testRealSignalInterruptedSendIsRetried) {
   const ChildResult r = runInChild(checkRealInterruptedSend, TIME_LIMIT_MILLIS);
   JUDGE_CHILD("real signal interrupts a blocked send", r);
}

//******************************************************************************
// the test hook itself

POIVRE_TEST_CASE(TestSocketSendInterrupt, testSendOverrideIsRestored) {
   require(&detail::sendIoOps() == &detail::systemSendIoOps(), "the send paths use the real send() by default");
   {
      SendOverride outer(&SCRIPTED_OPS);
      require(&detail::sendIoOps() == &SCRIPTED_OPS, "an override takes effect");
      {
         SendOverride inner(&COUNTING_OPS);
         require(&detail::sendIoOps() == &COUNTING_OPS, "a nested override takes effect");
      }
      require(&detail::sendIoOps() == &SCRIPTED_OPS, "a nested override gives back the outer one");
   }
   require(&detail::sendIoOps() == &detail::systemSendIoOps(), "the real send() is back after the overrides");
   require(detail::setSendIoOps(nullptr) == &detail::systemSendIoOps(), "nullptr selects the real send()");
}
