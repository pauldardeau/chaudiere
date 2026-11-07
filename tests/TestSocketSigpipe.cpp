// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

// Tests that writing to a disconnected peer fails instead of terminating the process
// with SIGPIPE (defect D-7), through every write path of Socket.
//
// How the signal is detected: most checks install a counting SIGPIPE handler
// (SigpipeCounter) for their duration, so that a signal that is raised is counted and
// the test process survives to report it; the previous disposition is restored when
// the counter goes out of scope. The checks that matter most for the real outcome run
// in a forked child with SIGPIPE at its *default* disposition (explicitly: the test
// runner may have inherited SIG_IGN, which would hide the defect) and require that the
// child exits normally instead of being killed by signal 13. Children have no threads
// and a hard time limit (see ChildProcess.h).
//
// The "disconnected peer": a loopback TCP connection whose other end has been closed.
// The first write afterwards is accepted by the kernel (and provokes a reset); a later
// one fails with EPIPE, which raises SIGPIPE unless the send suppresses it. Every check
// therefore writes several times, and requires that writes fail from some point on.
//
// Ports are chosen by the kernel (bind to port 0).

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>

#include "TestSocketSigpipe.h"
#include "ChildProcess.h"
#include "BasicException.h"
#include "ServerSocket.h"
#include "Socket.h"

using namespace chaudiere;

//******************************************************************************

TestSocketSigpipe::TestSocketSigpipe() :
   poivre::TestSuite("TestSocketSigpipe") {
}

//******************************************************************************

namespace {

using namespace chaudiere_test;

void sleepMillis(int millis) {
   std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}

// ---------------------------------------------------------------- SIGPIPE counting

volatile sig_atomic_t g_sigpipes = 0;

void onSigpipe(int) {
   g_sigpipes = g_sigpipes + 1;
}

// Counts the SIGPIPEs raised while it exists; restores the previous disposition (and
// signal mask is untouched) when it goes out of scope.
class SigpipeCounter {
public:
   SigpipeCounter() : m_installed(false) {
      g_sigpipes = 0;
      struct sigaction action;
      ::memset(&action, 0, sizeof(action));
      action.sa_handler = onSigpipe;
      ::sigemptyset(&action.sa_mask);
      action.sa_flags = 0;
      m_installed = (::sigaction(SIGPIPE, &action, &m_previous) == 0);
   }

   ~SigpipeCounter() {
      if (m_installed) {
         ::sigaction(SIGPIPE, &m_previous, nullptr);
      }
   }

   SigpipeCounter(const SigpipeCounter&) = delete;
   SigpipeCounter& operator=(const SigpipeCounter&) = delete;

   bool installed() const { return m_installed; }
   int count() const { return static_cast<int>(g_sigpipes); }

private:
   bool m_installed;
   struct sigaction m_previous;
};

// ---------------------------------------------------------------- loopback connections

// a listening socket on the loopback interface, port chosen by the kernel
struct Listener {
   int fd;
   int port;

   Listener() : fd(-1), port(0) {
      fd = ::socket(AF_INET, SOCK_STREAM, 0);
      if (fd < 0) {
         return;
      }
      sockaddr_in addr;
      ::memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      addr.sin_port = 0;
      socklen_t len = sizeof(addr);
      if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 ||
          ::listen(fd, 8) != 0 ||
          ::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
         ::close(fd);
         fd = -1;
         return;
      }
      port = ntohs(addr.sin_port);
   }

   ~Listener() {
      if (fd >= 0) {
         ::close(fd);
      }
   }

   Listener(const Listener&) = delete;
   Listener& operator=(const Listener&) = delete;

   bool ok() const { return fd >= 0; }
};

int connectRaw(int port) {
   const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0) {
      return -1;
   }
   sockaddr_in addr;
   ::memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   addr.sin_port = htons(port);
   if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
      ::close(fd);
      return -1;
   }
   return fd;
}

// waits until the peer's closing has had time to reach the other end
void letTheCloseArrive() {
   sleepMillis(60);
}

// A Socket made with Socket(address, port) whose peer has closed its end.
struct ClientOfClosedPeer {
   Listener listener;
   std::unique_ptr<Socket> socket;

   ClientOfClosedPeer() {
      if (!listener.ok()) {
         return;
      }
      try {
         socket.reset(new Socket("127.0.0.1", listener.port));
      } catch (const BasicException&) {
         return;
      }
      const int accepted = ::accept(listener.fd, nullptr, nullptr);
      if (accepted >= 0) {
         ::close(accepted);                 // the peer disconnects
      }
      letTheCloseArrive();
   }

   bool ok() const { return socket != nullptr; }
};

// A Socket that adopts an accepted descriptor (Socket(int) or Socket(observer, int)),
// whose peer has closed its end.
struct AdoptedSocketOfClosedPeer {
   Listener listener;
   std::unique_ptr<Socket> socket;

   explicit AdoptedSocketOfClosedPeer(bool withObserverConstructor) {
      if (!listener.ok()) {
         return;
      }
      const int client = connectRaw(listener.port);
      const int accepted = ::accept(listener.fd, nullptr, nullptr);
      if (client < 0 || accepted < 0) {
         if (client >= 0) ::close(client);
         if (accepted >= 0) ::close(accepted);
         return;
      }
      if (withObserverConstructor) {
         socket.reset(new Socket(static_cast<SocketCompletionObserver*>(nullptr), accepted));
      } else {
         socket.reset(new Socket(accepted));
      }
      ::close(client);                      // the peer disconnects
      letTheCloseArrive();
   }

   bool ok() const { return socket != nullptr; }
};

// ---------------------------------------------------------------- the write loop

struct WriteOutcome {
   int attempts = 0;
   int succeeded = 0;
   int failed = 0;
   bool succeededAfterFailure = false;   // a write succeeded after an earlier one had failed
};

// Makes `attempts` writes, 30 ms apart, with whatever `doWrite` does. The first write
// after the peer's close may succeed; once one has failed, they all must.
template <class Fn>
WriteOutcome writeRepeatedly(int attempts, Fn doWrite) {
   WriteOutcome outcome;
   for (int i = 0; i < attempts; ++i) {
      ++outcome.attempts;
      if (doWrite()) {
         ++outcome.succeeded;
         if (outcome.failed > 0) {
            outcome.succeededAfterFailure = true;
         }
      } else {
         ++outcome.failed;
      }
      sleepMillis(30);
   }
   return outcome;
}

std::string outcomeText(const WriteOutcome& o, int sigpipes) {
   return std::to_string(o.succeeded) + " of " + std::to_string(o.attempts) +
          " writes succeeded, " + std::to_string(o.failed) + " failed, SIGPIPE raised " +
          std::to_string(sigpipes) + " time(s)";
}

const char PAYLOAD[] = "0123456789abcdef0123456789abcdef";
const unsigned long PAYLOAD_LENGTH = sizeof(PAYLOAD) - 1;

// the MSG_NOSIGNAL a caller may pass to Socket::send (0 where the platform has none)
#ifdef MSG_NOSIGNAL
const int CALLER_NOSIGNAL = MSG_NOSIGNAL;
#else
const int CALLER_NOSIGNAL = 0;
#endif

const int TIME_LIMIT_MILLIS = 20000;

// what a passing check of a disconnected peer must satisfy
#define REQUIRE_PROTECTED(outcome, counter, what) \
   require((counter).count() == 0, std::string(what) + ": SIGPIPE was raised: " + outcomeText((outcome), (counter).count())); \
   require((outcome).failed >= 1, std::string(what) + ": no write to the disconnected peer failed: " + outcomeText((outcome), (counter).count())); \
   requireFalse((outcome).succeededAfterFailure, std::string(what) + ": a write succeeded after a write had failed")

// ---------------------------------------------------------------- checks run in a child
// (default SIGPIPE disposition, set explicitly)

int childWritesToClosedPeer(FILE* note, int variant) {
   ::signal(SIGPIPE, SIG_DFL);

   ClientOfClosedPeer peer;
   if (!peer.ok()) {
      fprintf(note, "could not set the connection up");
      return CHILD_SETUP_FAILED;
   }
   if (variant >= 3) {
      peer.socket->setIncludeMessageSize(true);
   }

   const WriteOutcome o = writeRepeatedly(5, [&]() {
      switch (variant) {
         case 0: return peer.socket->write(PAYLOAD, PAYLOAD_LENGTH);
         case 1: return peer.socket->write(std::string(PAYLOAD));
         case 2: return peer.socket->send(PAYLOAD, PAYLOAD_LENGTH, 0) > 0;
         case 3: return peer.socket->write(std::string(PAYLOAD));
         default: return peer.socket->send(PAYLOAD, PAYLOAD_LENGTH, CALLER_NOSIGNAL) > 0;
      }
   });

   // still alive: the process was not killed by SIGPIPE
   fprintf(note, "survived: %d of %d writes succeeded, %d failed", o.succeeded, o.attempts, o.failed);
   if (o.failed < 1) {
      return CHILD_CHECK_FAILED;
   }
   return o.succeededAfterFailure ? CHILD_CHECK_FAILED : CHILD_OK;
}

// writes through a Socket returned by ServerSocket::accept()
int childWritesToClosedPeerOfAcceptedSocket(FILE* note, int port) {
   ::signal(SIGPIPE, SIG_DFL);

   ServerSocket server(port);
   const int client = connectRaw(port);
   if (client < 0) {
      fprintf(note, "could not connect to the ServerSocket");
      return CHILD_SETUP_FAILED;
   }
   std::unique_ptr<Socket> accepted(server.accept());
   if (!accepted) {
      fprintf(note, "ServerSocket::accept() returned nothing");
      ::close(client);
      return CHILD_SETUP_FAILED;
   }
   ::close(client);                         // the peer disconnects
   letTheCloseArrive();

   const WriteOutcome o = writeRepeatedly(5, [&]() { return accepted->write(PAYLOAD, PAYLOAD_LENGTH); });
   fprintf(note, "survived: %d of %d writes succeeded, %d failed", o.succeeded, o.attempts, o.failed);
   return (o.failed >= 1 && !o.succeededAfterFailure) ? CHILD_OK : CHILD_CHECK_FAILED;
}

// a caller's own flags must still reach send(): MSG_DONTWAIT on a connection whose peer is
// not reading must fail with "would block" instead of blocking the writer for ever
int childCallerFlagsAreHonoured(FILE* note) {
   ::signal(SIGPIPE, SIG_DFL);

   Listener listener;
   if (!listener.ok()) {
      fprintf(note, "could not listen");
      return CHILD_SETUP_FAILED;
   }
   std::unique_ptr<Socket> socket;
   try {
      socket.reset(new Socket("127.0.0.1", listener.port));
   } catch (const BasicException&) {
      fprintf(note, "could not connect");
      return CHILD_SETUP_FAILED;
   }
   const int accepted = ::accept(listener.fd, nullptr, nullptr);   // and never reads
   if (accepted < 0) {
      fprintf(note, "could not accept");
      return CHILD_SETUP_FAILED;
   }

   // far more than the loopback buffers can hold
   const std::string big(64u * 1024u * 1024u, 'z');
   const ssize_t sent = socket->send(big.data(), big.size(), MSG_DONTWAIT);
   ::close(accepted);

   if (sent > 0) {
      fprintf(note, "a 64 MiB send with MSG_DONTWAIT to a peer that never reads reported success");
      return CHILD_CHECK_FAILED;
   }
   fprintf(note, "the send returned %zd at once instead of blocking", sent);
   return CHILD_OK;
}

}

//******************************************************************************
// writes to a disconnected peer, one test case per write path (SIGPIPE counted)

POIVRE_TEST_CASE(TestSocketSigpipe, testWriteBufferToDisconnectedPeerRaisesNoSigpipe) {
   ClientOfClosedPeer peer;
   require(peer.ok(), "a connection to a loopback listener could be made");
   SigpipeCounter counter;
   require(counter.installed(), "the counting SIGPIPE handler could be installed");

   const WriteOutcome o = writeRepeatedly(5, [&]() { return peer.socket->write(PAYLOAD, PAYLOAD_LENGTH); });
   REQUIRE_PROTECTED(o, counter, "Socket::write(buffer, length)");
}

POIVRE_TEST_CASE(TestSocketSigpipe, testWriteStringToDisconnectedPeerRaisesNoSigpipe) {
   ClientOfClosedPeer peer;
   require(peer.ok(), "a connection to a loopback listener could be made");
   SigpipeCounter counter;
   require(counter.installed(), "the counting SIGPIPE handler could be installed");

   const WriteOutcome o = writeRepeatedly(5, [&]() { return peer.socket->write(std::string(PAYLOAD)); });
   REQUIRE_PROTECTED(o, counter, "Socket::write(std::string)");
}

POIVRE_TEST_CASE(TestSocketSigpipe, testSendWithZeroFlagsToDisconnectedPeerRaisesNoSigpipe) {
   ClientOfClosedPeer peer;
   require(peer.ok(), "a connection to a loopback listener could be made");
   SigpipeCounter counter;
   require(counter.installed(), "the counting SIGPIPE handler could be installed");

   const WriteOutcome o = writeRepeatedly(5, [&]() { return peer.socket->send(PAYLOAD, PAYLOAD_LENGTH, 0) > 0; });
   REQUIRE_PROTECTED(o, counter, "Socket::send(buffer, length, 0)");
}

POIVRE_TEST_CASE(TestSocketSigpipe, testSendWithCallerFlagsToDisconnectedPeerRaisesNoSigpipe) {
   SigpipeCounter counter;
   require(counter.installed(), "the counting SIGPIPE handler could be installed");

   // the caller's own MSG_NOSIGNAL (where the platform has it), and a flag that has nothing to
   // do with signals: neither may re-enable the signal
   {
      ClientOfClosedPeer peer;
      require(peer.ok(), "a connection to a loopback listener could be made");
      const WriteOutcome o = writeRepeatedly(5, [&]() { return peer.socket->send(PAYLOAD, PAYLOAD_LENGTH, CALLER_NOSIGNAL) > 0; });
      REQUIRE_PROTECTED(o, counter, "Socket::send(buffer, length, MSG_NOSIGNAL)");
   }
   {
      ClientOfClosedPeer peer;
      require(peer.ok(), "a connection to a loopback listener could be made");
      const WriteOutcome o = writeRepeatedly(5, [&]() { return peer.socket->send(PAYLOAD, PAYLOAD_LENGTH, MSG_DONTWAIT) > 0; });
      REQUIRE_PROTECTED(o, counter, "Socket::send(buffer, length, MSG_DONTWAIT)");
   }
}

POIVRE_TEST_CASE(TestSocketSigpipe, testCallerFlagsAreStillHonouredByTheSend) {
   // MSG_DONTWAIT must still reach send(): the send fails at once instead of blocking
   const ChildResult r = runInChild([](FILE* note) { return childCallerFlagsAreHonoured(note); }, TIME_LIMIT_MILLIS);
   require(r.exited && (r.exitCode == CHILD_OK), describe("caller-supplied flags", r));
}

POIVRE_TEST_CASE(TestSocketSigpipe, testSizePrefixedSendToDisconnectedPeerRaisesNoSigpipe) {
   SigpipeCounter counter;
   require(counter.installed(), "the counting SIGPIPE handler could be installed");

   // the 2-byte size prefix is a send of its own
   {
      ClientOfClosedPeer peer;
      require(peer.ok(), "a connection to a loopback listener could be made");
      peer.socket->setIncludeMessageSize(true);
      const WriteOutcome o = writeRepeatedly(5, [&]() { return peer.socket->write(std::string(PAYLOAD)); });
      REQUIRE_PROTECTED(o, counter, "size-prefixed Socket::write");
   }
   {
      ClientOfClosedPeer peer;
      require(peer.ok(), "a connection to a loopback listener could be made");
      peer.socket->setIncludeMessageSize(true);
      const WriteOutcome o = writeRepeatedly(5, [&]() { return peer.socket->send(PAYLOAD, PAYLOAD_LENGTH, CALLER_NOSIGNAL) > 0; });
      REQUIRE_PROTECTED(o, counter, "size-prefixed Socket::send with the caller's MSG_NOSIGNAL");
   }
}

POIVRE_TEST_CASE(TestSocketSigpipe, testWriteFromWorkerThreadRaisesNoSigpipe) {
   ClientOfClosedPeer peer;
   require(peer.ok(), "a connection to a loopback listener could be made");
   SigpipeCounter counter;
   require(counter.installed(), "the counting SIGPIPE handler could be installed");

   WriteOutcome o;
   std::thread worker([&]() {
      o = writeRepeatedly(5, [&]() { return peer.socket->write(std::string(PAYLOAD)); });
   });
   worker.join();
   REQUIRE_PROTECTED(o, counter, "Socket::write on a worker thread");
}

POIVRE_TEST_CASE(TestSocketSigpipe, testAdoptedDescriptorsAreProtected) {
   SigpipeCounter counter;
   require(counter.installed(), "the counting SIGPIPE handler could be installed");

   // descriptors handed to the constructors (an accepted connection, say), whose peer then
   // closes; a Socket must not rely on the descriptor having been set up for it
   {
      AdoptedSocketOfClosedPeer peer(false);
      require(peer.ok(), "an accepted connection could be made");
      const WriteOutcome o = writeRepeatedly(5, [&]() { return peer.socket->write(PAYLOAD, PAYLOAD_LENGTH); });
      REQUIRE_PROTECTED(o, counter, "Socket(int)");
   }
   {
      AdoptedSocketOfClosedPeer peer(true);
      require(peer.ok(), "an accepted connection could be made");
      const WriteOutcome o = writeRepeatedly(5, [&]() { return peer.socket->write(PAYLOAD, PAYLOAD_LENGTH); });
      REQUIRE_PROTECTED(o, counter, "Socket(observer, int)");
   }
}

POIVRE_TEST_CASE(TestSocketSigpipe, testSocketAdoptedAfterThePeerHasGoneStillReadsAndRaisesNoSigpipe) {
   // A one-way message: the peer sends its request and closes at once, and the connection is
   // accepted only afterwards. On some platforms the descriptor of such a connection can no
   // longer be configured (setsockopt(SO_NOSIGPIPE) may fail), and then the Socket must neither
   // lose the request nor risk SIGPIPE: reading works, and a write fails (or is refused).
   Listener listener;
   require(listener.ok(), "a loopback listener could be made");
   const int client = connectRaw(listener.port);
   require(client >= 0, "a client could connect");
   require(::send(client, "request", 7, 0) == 7, "the client sent its request");
   ::close(client);                         // the peer is gone before the accept
   letTheCloseArrive();
   const int accepted = ::accept(listener.fd, nullptr, nullptr);
   require(accepted >= 0, "the connection could be accepted");

   SigpipeCounter counter;
   require(counter.installed(), "the counting SIGPIPE handler could be installed");

   Socket socket(accepted);
   char buffer[16];
   ::memset(buffer, 0, sizeof(buffer));
   require(socket.readSocket(buffer, 7) == 7, "the request sent before the peer closed can still be read");
   requireStringEquals("request", std::string(buffer, 7), "the request is intact");

   const WriteOutcome o = writeRepeatedly(5, [&]() { return socket.write(PAYLOAD, PAYLOAD_LENGTH); });
   REQUIRE_PROTECTED(o, counter, "a write to a connection adopted after its peer had gone");
}

POIVRE_TEST_CASE(TestSocketSigpipe, testRepeatedDisconnectedPeerWritesRaiseNoSigpipe) {
   SigpipeCounter counter;
   require(counter.installed(), "the counting SIGPIPE handler could be installed");

   int totalFailed = 0;
   for (int i = 0; i < 15; ++i) {
      ClientOfClosedPeer peer;
      require(peer.ok(), "a connection to a loopback listener could be made");
      const WriteOutcome o = writeRepeatedly(3, [&]() { return peer.socket->write(PAYLOAD, PAYLOAD_LENGTH); });
      totalFailed += o.failed;
   }
   require(counter.count() == 0, "15 disconnected peers: SIGPIPE was raised " + std::to_string(counter.count()) + " time(s)");
   require(totalFailed >= 15, "writes to the disconnected peers failed (" + std::to_string(totalFailed) + " failures)");
}

//******************************************************************************
// the real outcome: a process with SIGPIPE at its default disposition survives

POIVRE_TEST_CASE(TestSocketSigpipe, testProcessWithDefaultDispositionSurvivesEveryWritePath) {
   const char* names[] = { "Socket::write(buffer, length)", "Socket::write(std::string)",
                           "Socket::send(buffer, length, 0)", "size-prefixed Socket::write",
                           "size-prefixed Socket::send with MSG_NOSIGNAL" };

   for (int variant = 0; variant < 5; ++variant) {
      const ChildResult r = runInChild([variant](FILE* note) { return childWritesToClosedPeer(note, variant); },
                                       TIME_LIMIT_MILLIS);
      require(r.exited && (r.exitCode == CHILD_OK), describe(names[variant], r));
   }
}

POIVRE_TEST_CASE(TestSocketSigpipe, testAcceptedSocketOfServerSocketSurvivesDisconnectedPeer) {
   // a port the kernel has just handed out; ServerSocket binds it in the child
   Listener probe;
   require(probe.ok(), "a port could be chosen");
   const int port = probe.port;
   ::close(probe.fd);
   probe.fd = -1;

   const ChildResult r = runInChild([port](FILE* note) { return childWritesToClosedPeerOfAcceptedSocket(note, port); },
                                    TIME_LIMIT_MILLIS);
   require(r.exited && (r.exitCode == CHILD_OK), describe("a Socket returned by ServerSocket::accept()", r));
}

//******************************************************************************
// successful writes to a connected peer are unchanged

namespace {

// reads exactly `count` bytes from fd, waiting at most `timeoutMillis` in all
std::string readExactly(int fd, size_t count, int timeoutMillis) {
   std::string data;
   const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
   while (data.size() < count) {
      const long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
         std::chrono::steady_clock::now() - start).count();
      if (elapsed >= timeoutMillis) {
         break;
      }
      pollfd p;
      p.fd = fd;
      p.events = POLLIN;
      p.revents = 0;
      if (::poll(&p, 1, static_cast<int>(timeoutMillis - elapsed)) <= 0) {
         break;
      }
      char buffer[256];
      const size_t want = (count - data.size() < sizeof(buffer)) ? (count - data.size()) : sizeof(buffer);
      const ssize_t n = ::recv(fd, buffer, want, 0);
      if (n <= 0) {
         break;
      }
      data.append(buffer, static_cast<size_t>(n));
   }
   return data;
}

}

POIVRE_TEST_CASE(TestSocketSigpipe, testSuccessfulWritesToConnectedPeerAreUnchanged) {
   Listener listener;
   require(listener.ok(), "a loopback listener could be made");
   std::unique_ptr<Socket> socket;
   try {
      socket.reset(new Socket("127.0.0.1", listener.port));
   } catch (const BasicException&) {
   }
   require(socket != nullptr, "a connection could be made");
   const int peer = ::accept(listener.fd, nullptr, nullptr);
   require(peer >= 0, "the connection could be accepted");

   SigpipeCounter counter;

   require(socket->write(std::string(PAYLOAD)), "write(std::string) to a connected peer succeeds");
   requireStringEquals(std::string(PAYLOAD), readExactly(peer, PAYLOAD_LENGTH, 5000), "write(std::string) delivers its bytes");

   require(socket->write(PAYLOAD, PAYLOAD_LENGTH), "write(buffer, length) to a connected peer succeeds");
   requireStringEquals(std::string(PAYLOAD), readExactly(peer, PAYLOAD_LENGTH, 5000), "write(buffer, length) delivers its bytes");

   require(socket->send(PAYLOAD, PAYLOAD_LENGTH, 0) == static_cast<ssize_t>(PAYLOAD_LENGTH), "send(buffer, length, 0) returns the length");
   requireStringEquals(std::string(PAYLOAD), readExactly(peer, PAYLOAD_LENGTH, 5000), "send(buffer, length, 0) delivers its bytes");

   require(socket->send(PAYLOAD, PAYLOAD_LENGTH, CALLER_NOSIGNAL) == static_cast<ssize_t>(PAYLOAD_LENGTH), "send with the caller's flags returns the length");
   requireStringEquals(std::string(PAYLOAD), readExactly(peer, PAYLOAD_LENGTH, 5000), "send with the caller's flags delivers its bytes");

   // size-prefixed: a 2-byte big-endian length, then the payload, in that order
   socket->setIncludeMessageSize(true);
   require(socket->write(std::string(PAYLOAD)), "a size-prefixed write to a connected peer succeeds");
   const std::string framed = readExactly(peer, 2 + PAYLOAD_LENGTH, 5000);
   require(framed.size() == 2 + PAYLOAD_LENGTH, "the size prefix and the payload arrive");
   if (framed.size() == 2 + PAYLOAD_LENGTH) {
      require((static_cast<unsigned char>(framed[0]) == 0) &&
              (static_cast<unsigned char>(framed[1]) == PAYLOAD_LENGTH),
              "the size prefix is the payload length, big-endian");
      requireStringEquals(std::string(PAYLOAD), framed.substr(2), "the framed payload follows the prefix");
   }

   require(counter.count() == 0, "no SIGPIPE while writing to a connected peer");
   ::close(peer);
}
