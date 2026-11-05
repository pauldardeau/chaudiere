// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <exception>
#include <string>

#include "TestKqueueServer.h"
#include "KqueueServer.h"
#include "PthreadsMutex.h"
#include "PthreadsThread.h"
#include "SocketServiceHandler.h"
#include "SocketRequest.h"
#include "Socket.h"
#include "Runnable.h"
#include "Thread.h"

using namespace chaudiere;

namespace {

// How long a supported-platform test waits for the kernel to report an event
// that has already been made ready, and how often it checks. These are
// ceilings, not delays: a working kqueue answers at once.
const long EVENT_WAIT_TIMEOUT_MILLIS = 5000;
const long EVENT_WAIT_POLL_MILLIS = 10;

// the size of the server's event array; getKernelEvents() is never asked for
// more events than this
const int MAX_CONNECTIONS = 10;

// Unused by these tests directly, but required to initialize a KqueueServer.
class NoOpSocketServiceHandler : public chaudiere::SocketServiceHandler {
public:
   void serviceSocket(chaudiere::SocketRequest*) override {
   }

   const std::string& getName() const override {
      static const std::string name = "NoOpSocketServiceHandler";
      return name;
   }
};

// KqueueServer with the listener socket's descriptor made visible, so that a
// test can tell which returned event is the listener's.
class InspectableKqueueServer : public KqueueServer {
public:
   InspectableKqueueServer(Mutex& fdMutex, Mutex& hwmMutex) :
      KqueueServer(fdMutex, hwmMutex) {
   }

   int listenerFileDescriptor() const {
      return getListenerSocketFileDescriptor();
   }
};

// Two connected local sockets: whatever is written to one becomes readable on
// the other, and closing one is end-of-stream for the other. The descriptors
// are closed when the object goes out of scope.
class SocketPair {
public:
   SocketPair() :
      m_first(-1),
      m_second(-1) {
      int fds[2];
      if (0 == ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds)) {
         m_first = fds[0];
         m_second = fds[1];
      }
   }

   ~SocketPair() {
      closeFirst();
      closeSecond();
   }

   bool isValid() const {
      return (m_first != -1) && (m_second != -1);
   }

   int first() const {
      return m_first;
   }

   int second() const {
      return m_second;
   }

   void closeFirst() {
      if (m_first != -1) {
         ::close(m_first);
         m_first = -1;
      }
   }

   void closeSecond() {
      if (m_second != -1) {
         ::close(m_second);
         m_second = -1;
      }
   }

   bool writeToSecond() {
      const char byte = 'x';
      return (m_second != -1) && (1 == ::write(m_second, &byte, 1));
   }

private:
   int m_first;
   int m_second;

   // copying not allowed
   SocketPair(const SocketPair&);
   SocketPair& operator=(const SocketPair&);
};

// Drives KqueueServer::getKernelEvents(), which has no timeout and blocks
// until a descriptor it watches is ready, on a background thread.
class GetEventsRunnable : public chaudiere::Runnable {
public:
   GetEventsRunnable(KqueueServer& server, int maxEvents) :
      result(-1),
      m_server(server),
      m_maxEvents(maxEvents),
      m_done(false) {
   }

   void run() override {
      result = m_server.getKernelEvents(m_maxEvents);
      m_done = true;
   }

   bool isDone() const {
      return m_done;
   }

   // only to be read once the thread has been joined
   int result;

private:
   KqueueServer& m_server;
   const int m_maxEvents;
   std::atomic<bool> m_done;
};

struct EventWait {
   EventWait() :
      numEvents(-1),
      delivered(false) {
   }

   // what getKernelEvents() returned
   int numEvents;

   // false if the wait timed out (or could not be started); numEvents then
   // only counts the wake-up event and must not be examined
   bool delivered;
};

// Calls server.getKernelEvents() on a background thread and waits for it for
// at most EVENT_WAIT_TIMEOUT_MILLIS. The caller makes an event ready first, so
// a working kqueue answers at once. If it does not, a byte is written to
// wakeWriteFD -- the far end of a SocketPair whose near end the caller has
// registered with the server -- so that the blocked call returns anyway and
// the thread is always joined before this function returns.
EventWait waitForEvents(KqueueServer& server, int wakeWriteFD) {
   EventWait wait;

   GetEventsRunnable getEvents(server, MAX_CONNECTIONS);
   PthreadsThread getEventsThread(&getEvents);
   if (!getEventsThread.start()) {
      return wait;
   }

   long waitedMillis = 0;
   while (!getEvents.isDone() && (waitedMillis < EVENT_WAIT_TIMEOUT_MILLIS)) {
      Thread::sleep(EVENT_WAIT_POLL_MILLIS);
      waitedMillis += EVENT_WAIT_POLL_MILLIS;
   }

   wait.delivered = getEvents.isDone();

   if (!wait.delivered) {
      const char byte = 'x';
      if (1 != ::write(wakeWriteFD, &byte, 1)) {
         // the wake-up could not be sent. Nothing else can unblock the
         // thread, and destroying it while it still runs is worse than
         // waiting, so fall through to the join.
      }
   }

   getEventsThread.join();
   wait.numEvents = getEvents.result;

   return wait;
}

// Index of the returned event that belongs to fd, or -1. Only to be called
// with the count that a successful getKernelEvents() returned, so that every
// index below it is valid.
int indexOfEventForDescriptor(KqueueServer& server, int numEvents, int fd) {
   for (int index = 0; index < numEvents; ++index) {
      if (fd == server.fileDescriptorForEventIndex(index)) {
         return index;
      }
   }

   return -1;
}

}

//******************************************************************************

TestKqueueServer::TestKqueueServer() :
   poivre::TestSuite("TestKqueueServer") {
}

//******************************************************************************

void TestKqueueServer::runTests() {
   testIsSupportedPlatform();
   testConstructor();

   if (KqueueServer::isSupportedPlatform()) {
      testNotifySocketComplete();
      testSupportedInit();
      testSupportedFileDescriptorRegistration();
      testSupportedReadableDescriptorEvent();
      testSupportedListenerEvent();
      testSupportedEndOfStreamEvent();
      testSupportedRemovedDescriptorIsNotReported();
   } else {
      testInit();
      testRun();
      testNotifySocketComplete();
      testUnsupportedPlatformBehavior();
      testUnsupportedInitWithValidHandler();
   }
}

//******************************************************************************

void TestKqueueServer::testIsSupportedPlatform() {
   TEST_CASE("testIsSupportedPlatform");

   // kqueue is only available on BSD/macOS. runTests() chooses its path from
   // this same answer, so it is also checked against the platform itself:
   // a platform that has kqueue must fail here, not be quietly routed to the
   // tests for platforms without it (which call the event accessors of a
   // server that was never initialized).
#if defined(__APPLE__)
   require(KqueueServer::isSupportedPlatform(), "kqueue should be reported as supported on this platform");
#elif defined(__linux__)
   requireFalse(KqueueServer::isSupportedPlatform(), "kqueue should not be reported as supported on this platform");
#else
   // the other BSDs: no expectation of the answer is recorded here
#endif
}

//******************************************************************************

void TestKqueueServer::testConstructor() {
   TEST_CASE("testConstructor");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   KqueueServer server(fdMutex, hwmMutex);
   require(true, "constructing a KqueueServer should not throw, even on an unsupported platform");
}

//******************************************************************************

void TestKqueueServer::testInit() {
   TEST_CASE("testInit");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   KqueueServer server(fdMutex, hwmMutex);

   // without kqueue support, init() returns false immediately without
   // touching socketServiceHandler, so nullptr is safe here
   requireFalse(server.init(nullptr, 9999, 10), "init should fail on a platform without kqueue support");
}

//******************************************************************************

void TestKqueueServer::testRun() {
   //TEST_CASE("testRun");
   //TODO: run() (inherited from KernelEventServer) loops calling
   // getKernelEvents() forever when it returns a value less than 1, which
   // it always does on a platform without kqueue support -- there's no way
   // to safely exercise this without hanging the test process.
}

//******************************************************************************

void TestKqueueServer::testNotifySocketComplete() {
   TEST_CASE("testNotifySocketComplete");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   KqueueServer server(fdMutex, hwmMutex);

   // notifySocketComplete() returns immediately for a socket with an
   // invalid file descriptor, which is the only path safely reachable
   // without a fully initialized kernel event server
   Socket socket(-1);
   server.notifySocketComplete(&socket);
   require(true, "notifySocketComplete should not throw for a socket with an invalid file descriptor");
}

//******************************************************************************

void TestKqueueServer::testUnsupportedPlatformBehavior() {
   TEST_CASE("testUnsupportedPlatformBehavior");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   KqueueServer server(fdMutex, hwmMutex);

   // every kqueue-specific operation degrades to a safe, well-defined
   // no-op/failure value on a platform without kqueue support.
   // These calls are only safe there: with kqueue, the event accessors read
   // the event array that init() allocates, which an uninitialized server
   // does not have (they crashed on macOS before runTests() was split).
   require(0 == server.getKernelEvents(10), "getKernelEvents should return 0 without kqueue support");
   require(-1 == server.fileDescriptorForEventIndex(0), "fileDescriptorForEventIndex should return -1 without kqueue support");
   requireFalse(server.addFileDescriptorForRead(0), "addFileDescriptorForRead should fail without kqueue support");
   requireFalse(server.removeFileDescriptorFromRead(0), "removeFileDescriptorFromRead should fail without kqueue support");
   requireFalse(server.isEventDisconnect(0), "isEventDisconnect should return false without kqueue support");
   requireFalse(server.isEventReadClose(0), "isEventReadClose should return false without kqueue support");
   requireFalse(server.isEventRead(0), "isEventRead should return false without kqueue support");
}

//******************************************************************************

void TestKqueueServer::testUnsupportedInitWithValidHandler() {
   TEST_CASE("testUnsupportedInitWithValidHandler");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   KqueueServer server(fdMutex, hwmMutex);

   // testInit() gives init() no handler, which would fail on any platform.
   // Here the handler, port and connection limit are valid, so false can only
   // mean "unsupported". Without kqueue init() returns before taking ownership
   // of the handler and before opening a socket, so it is not heap-allocated.
   NoOpSocketServiceHandler handler;
   requireFalse(server.init(&handler, 9999, 10), "init should fail with a valid handler on a platform without kqueue support");
}

//******************************************************************************

// The tests below are for platforms with kqueue. Each one initializes its own
// server first and goes on only if that worked, makes the event it looks at
// ready before asking for events, and only passes fileDescriptorForEventIndex()
// and the isEvent*() calls an index that getKernelEvents() has returned.
// Events are made with local socket pairs (and one loopback connection), and
// every descriptor is closed when the test ends.

void TestKqueueServer::testSupportedInit() {
   TEST_CASE("testSupportedInit");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   InspectableKqueueServer server(fdMutex, hwmMutex);

   // ownership of the handler transfers to the server
   require(server.init(new NoOpSocketServiceHandler(), 44760, MAX_CONNECTIONS), "init should succeed with a valid handler, port, and connection limit");
   require(server.listenerFileDescriptor() >= 0, "init should have opened the listener socket");
}

//******************************************************************************

void TestKqueueServer::testSupportedFileDescriptorRegistration() {
   TEST_CASE("testSupportedFileDescriptorRegistration");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   InspectableKqueueServer server(fdMutex, hwmMutex);
   if (!server.init(new NoOpSocketServiceHandler(), 44761, MAX_CONNECTIONS)) {
      failTest("sanity check: init should succeed");
      return;
   }

   SocketPair pair;
   if (!pair.isValid()) {
      failTest("sanity check: creating a socket pair should succeed");
      return;
   }

   require(server.addFileDescriptorForRead(pair.first()), "addFileDescriptorForRead should succeed for a valid, not-yet-watched descriptor");
   require(server.removeFileDescriptorFromRead(pair.first()), "removeFileDescriptorFromRead should succeed for a currently-watched descriptor");
   requireFalse(server.removeFileDescriptorFromRead(pair.first()), "removeFileDescriptorFromRead should fail for a descriptor that is no longer watched");
}

//******************************************************************************

void TestKqueueServer::testSupportedReadableDescriptorEvent() {
   TEST_CASE("testSupportedReadableDescriptorEvent");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   InspectableKqueueServer server(fdMutex, hwmMutex);
   if (!server.init(new NoOpSocketServiceHandler(), 44762, MAX_CONNECTIONS)) {
      failTest("sanity check: init should succeed");
      return;
   }

   SocketPair wake;
   SocketPair data;
   if (!wake.isValid() || !data.isValid() ||
       !server.addFileDescriptorForRead(wake.first()) ||
       !server.addFileDescriptorForRead(data.first())) {
      failTest("sanity check: setting up the watched descriptors should succeed");
      return;
   }

   // data.first() becomes readable
   require(data.writeToSecond(), "sanity check: writing to the socket pair should succeed");

   const EventWait wait = waitForEvents(server, wake.second());
   require(wait.delivered, "getKernelEvents should report the readable descriptor within the time limit");
   if (wait.delivered) {
      require(wait.numEvents >= 1, "getKernelEvents should return at least one event");

      // other watched descriptors may be reported too, and in any order
      const int index = indexOfEventForDescriptor(server, wait.numEvents, data.first());
      require(index >= 0, "the readable descriptor should be among the returned events");
      require(-1 == indexOfEventForDescriptor(server, wait.numEvents, wake.first()), "a descriptor that is not ready should not be reported");
      if (index >= 0) {
         require(server.isEventRead(index), "data waiting to be read should be reported as a read event");
         requireFalse(server.isEventDisconnect(index), "a normal read event should not be reported as a disconnect");
         requireFalse(server.isEventReadClose(index), "a normal read event should not be reported as a read-close");
      }
   }

   require(server.removeFileDescriptorFromRead(data.first()), "removeFileDescriptorFromRead should succeed for the watched descriptor");
   require(server.removeFileDescriptorFromRead(wake.first()), "removeFileDescriptorFromRead should succeed for the watched descriptor");
}

//******************************************************************************

void TestKqueueServer::testSupportedListenerEvent() {
   TEST_CASE("testSupportedListenerEvent");

   const int port = 44763;
   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   InspectableKqueueServer server(fdMutex, hwmMutex);

   // init() registers the listener socket itself for read-readiness
   if (!server.init(new NoOpSocketServiceHandler(), port, MAX_CONNECTIONS)) {
      failTest("sanity check: init should succeed");
      return;
   }

   SocketPair wake;
   if (!wake.isValid() || !server.addFileDescriptorForRead(wake.first())) {
      failTest("sanity check: setting up the wake-up descriptor should succeed");
      return;
   }

   // a client that has connected makes the listener read-ready (accept-ready)
   // without anyone calling accept()
   try {
      Socket clientSocket("127.0.0.1", port);

      const EventWait wait = waitForEvents(server, wake.second());
      require(wait.delivered, "getKernelEvents should report the connection within the time limit");
      if (wait.delivered) {
         const int index = indexOfEventForDescriptor(server, wait.numEvents, server.listenerFileDescriptor());
         require(index >= 0, "the listener socket should be among the returned events once a client connects");
         if (index >= 0) {
            require(server.isEventRead(index), "the listener socket becoming accept-ready should be reported as a read event");
            requireFalse(server.isEventDisconnect(index), "a pending connection should not be reported as a disconnect");
            requireFalse(server.isEventReadClose(index), "a pending connection should not be reported as a read-close");
         }
      }
   } catch (const std::exception& e) {
      failTest(std::string("connecting to the listener threw: ") + e.what());
   }
}

//******************************************************************************

void TestKqueueServer::testSupportedEndOfStreamEvent() {
   TEST_CASE("testSupportedEndOfStreamEvent");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   InspectableKqueueServer server(fdMutex, hwmMutex);
   if (!server.init(new NoOpSocketServiceHandler(), 44764, MAX_CONNECTIONS)) {
      failTest("sanity check: init should succeed");
      return;
   }

   SocketPair wake;
   SocketPair data;
   if (!wake.isValid() || !data.isValid() ||
       !server.addFileDescriptorForRead(wake.first()) ||
       !server.addFileDescriptorForRead(data.first())) {
      failTest("sanity check: setting up the watched descriptors should succeed");
      return;
   }

   // the peer going away is what kqueue reports with its end-of-file flag;
   // KqueueServer maps that one flag to both isEventDisconnect() and
   // isEventReadClose()
   data.closeSecond();

   const EventWait wait = waitForEvents(server, wake.second());
   require(wait.delivered, "getKernelEvents should report the closed peer within the time limit");
   if (wait.delivered) {
      const int index = indexOfEventForDescriptor(server, wait.numEvents, data.first());
      require(index >= 0, "the descriptor whose peer closed should be among the returned events");
      if (index >= 0) {
         require(server.isEventRead(index), "end-of-stream is delivered as a read event");
         require(server.isEventDisconnect(index), "a closed peer should be reported as a disconnect");
         require(server.isEventReadClose(index), "a closed peer should be reported as a read-close");
      }
   }
}

//******************************************************************************

void TestKqueueServer::testSupportedRemovedDescriptorIsNotReported() {
   TEST_CASE("testSupportedRemovedDescriptorIsNotReported");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   InspectableKqueueServer server(fdMutex, hwmMutex);
   if (!server.init(new NoOpSocketServiceHandler(), 44765, MAX_CONNECTIONS)) {
      failTest("sanity check: init should succeed");
      return;
   }

   SocketPair wake;
   SocketPair data;
   if (!wake.isValid() || !data.isValid() ||
       !server.addFileDescriptorForRead(wake.first()) ||
       !server.addFileDescriptorForRead(data.first())) {
      failTest("sanity check: setting up the watched descriptors should succeed");
      return;
   }

   // data.first() is readable, but no longer watched. The wake-up descriptor
   // is made ready on purpose, so the call returns at once and the events it
   // does report can be looked at.
   require(data.writeToSecond(), "sanity check: writing to the socket pair should succeed");
   require(server.removeFileDescriptorFromRead(data.first()), "removeFileDescriptorFromRead should succeed for the watched descriptor");
   require(wake.writeToSecond(), "sanity check: writing to the wake-up socket pair should succeed");

   const EventWait wait = waitForEvents(server, wake.second());
   require(wait.delivered, "getKernelEvents should report the wake-up descriptor within the time limit");
   if (wait.delivered) {
      require(indexOfEventForDescriptor(server, wait.numEvents, wake.first()) >= 0, "the wake-up descriptor should be among the returned events");
      require(-1 == indexOfEventForDescriptor(server, wait.numEvents, data.first()), "a descriptor that is no longer watched should not be reported");
   }
}

//******************************************************************************
