// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <unistd.h>

#include "TestEpollServer.h"
#include "EpollServer.h"
#include "PthreadsMutex.h"
#include "PthreadsThread.h"
#include "SocketServiceHandler.h"
#include "SocketRequest.h"
#include "Socket.h"
#include "Runnable.h"
#include "Thread.h"

using namespace chaudiere;

namespace {

// Unused by these tests directly, but required to construct an EpollServer.
class NoOpSocketServiceHandler : public chaudiere::SocketServiceHandler {
public:
   void serviceSocket(chaudiere::SocketRequest*) override {
   }

   const std::string& getName() const override {
      static const std::string name = "NoOpSocketServiceHandler";
      return name;
   }
};

// Drives EpollServer::getKernelEvents() (which blocks until at least one
// fd it's watching becomes ready) on a background thread, so a test can
// trigger readiness (e.g. by connecting a real client) from the main
// thread without risking a permanent hang.
class GetEventsRunnable : public chaudiere::Runnable {
public:
   explicit GetEventsRunnable(EpollServer& server) :
      m_server(server),
      result(-1) {
   }

   void run() override {
      result = m_server.getKernelEvents(10);
   }

   int result;

private:
   EpollServer& m_server;
};

}

//******************************************************************************

TestEpollServer::TestEpollServer() :
   poivre::TestSuite("TestEpollServer") {
}

//******************************************************************************

void TestEpollServer::runTests() {
   testIsSupportedPlatform();
   testConstructor();

   if (EpollServer::isSupportedPlatform()) {
      testInitWithNullHandler();
      testInitWithInvalidPort();
      testInit();
      testAddFileDescriptorForRead();
      testRemoveFileDescriptorFromRead();
      testGetKernelEventsAndEventAccessors();
   } else {
      // none of the functional tests can work without epoll: they need a
      // successful init(), which opens a listener socket
      testUnsupportedInit();
      testUnsupportedFileDescriptorRegistration();
      testUnsupportedKernelEvents();
      testUnsupportedEventClassification();
   }
}

//******************************************************************************

void TestEpollServer::testIsSupportedPlatform() {
   TEST_CASE("testIsSupportedPlatform");

   // epoll is Linux-specific. runTests() chooses its path from this same
   // answer, so it is also checked against the platform itself: a Linux
   // build that lost epoll must fail here, not silently skip the epoll tests.
#ifdef __linux__
   require(EpollServer::isSupportedPlatform(), "epoll should be reported as supported on this platform");
#else
   requireFalse(EpollServer::isSupportedPlatform(), "epoll should be reported as unsupported on this platform");
#endif
}

//******************************************************************************

void TestEpollServer::testConstructor() {
   TEST_CASE("testConstructor");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);
   require(true, "constructing an EpollServer should not throw");
}

//******************************************************************************

void TestEpollServer::testInitWithNullHandler() {
   TEST_CASE("testInitWithNullHandler");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);

   requireFalse(server.init(nullptr, 44750, 10), "init should fail when given a null socket service handler");
}

//******************************************************************************

void TestEpollServer::testInitWithInvalidPort() {
   TEST_CASE("testInitWithInvalidPort");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);

   // ownership of the handler transfers to the server (via a unique_ptr)
   // even on failure paths, so it must be heap-allocated
   requireFalse(server.init(new NoOpSocketServiceHandler(), 0, 10), "init should fail for a non-positive port");
}

//******************************************************************************

void TestEpollServer::testInit() {
   TEST_CASE("testInit");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);

   require(server.init(new NoOpSocketServiceHandler(), 44751, 10), "init should succeed with a valid handler, port, and connection limit");
}

//******************************************************************************

void TestEpollServer::testAddFileDescriptorForRead() {
   TEST_CASE("testAddFileDescriptorForRead");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);
   require(server.init(new NoOpSocketServiceHandler(), 44752, 10), "sanity check: init should succeed");

   const int extraFD = Socket::createSocket();
   require(server.addFileDescriptorForRead(extraFD), "addFileDescriptorForRead should succeed for a valid, not-already-watched fd");

   ::close(extraFD);
}

//******************************************************************************

void TestEpollServer::testRemoveFileDescriptorFromRead() {
   TEST_CASE("testRemoveFileDescriptorFromRead");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);
   require(server.init(new NoOpSocketServiceHandler(), 44753, 10), "sanity check: init should succeed");

   const int extraFD = Socket::createSocket();
   server.addFileDescriptorForRead(extraFD);
   require(server.removeFileDescriptorFromRead(extraFD), "removeFileDescriptorFromRead should succeed for a currently-watched fd");

   ::close(extraFD);
}

//******************************************************************************

void TestEpollServer::testGetKernelEventsAndEventAccessors() {
   TEST_CASE("testGetKernelEventsAndEventAccessors");

   const int port = 44754;
   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);

   // init() registers the listener socket itself for read-readiness
   require(server.init(new NoOpSocketServiceHandler(), port, 10), "sanity check: init should succeed");

   GetEventsRunnable getEventsRunnable(server);
   PthreadsThread getEventsThread(&getEventsRunnable);
   require(getEventsThread.start(), "starting the getKernelEvents thread should succeed");

   // give the background thread a moment to actually reach epoll_wait()
   Thread::sleep(50);

   // connecting a client makes the listener fd read-ready (accept-ready),
   // which unblocks the epoll_wait() call running on the background thread
   Socket clientSocket("127.0.0.1", port);

   getEventsThread.join();

   require(getEventsRunnable.result >= 1, "getKernelEvents should return at least one ready event once a client connects");
   if (getEventsRunnable.result >= 1) {
      const int readyFD = server.fileDescriptorForEventIndex(0);
      require(readyFD >= 0, "fileDescriptorForEventIndex should return a valid file descriptor for the ready event");
      require(server.isEventRead(0), "the listener socket becoming accept-ready should be reported as a read event");
      requireFalse(server.isEventDisconnect(0), "a normal read-ready event should not be reported as a disconnect");
      requireFalse(server.isEventReadClose(0), "a normal read-ready event should not be reported as a read-close");
   }
}

//******************************************************************************

// The tests below are for platforms without epoll, where every EpollServer
// call other than the constructor returns a fixed value. They must only run
// there: on a platform with epoll the event accessors would dereference an
// event array that init() has not allocated.

void TestEpollServer::testUnsupportedInit() {
   TEST_CASE("testUnsupportedInit");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);

   // a valid handler, port and connection limit, so that false can only mean
   // "unsupported". Without epoll init() returns before taking ownership of
   // the handler (and before opening any socket), so it is not heap-allocated.
   NoOpSocketServiceHandler handler;
   requireFalse(server.init(&handler, 44750, 10), "init should fail on a platform without epoll");
}

//******************************************************************************

void TestEpollServer::testUnsupportedFileDescriptorRegistration() {
   TEST_CASE("testUnsupportedFileDescriptorRegistration");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);

   // stdin is only a valid descriptor number: without epoll the descriptor is
   // never looked at, and nothing is opened or closed here
   requireFalse(server.addFileDescriptorForRead(STDIN_FILENO), "addFileDescriptorForRead should fail on a platform without epoll");
   requireFalse(server.removeFileDescriptorFromRead(STDIN_FILENO), "removeFileDescriptorFromRead should fail on a platform without epoll");
}

//******************************************************************************

void TestEpollServer::testUnsupportedKernelEvents() {
   TEST_CASE("testUnsupportedKernelEvents");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);

   // returns immediately: nothing is waited for
   require(server.getKernelEvents(10) == 0, "getKernelEvents should report no events on a platform without epoll");
   require(server.fileDescriptorForEventIndex(0) == -1, "fileDescriptorForEventIndex should return -1 on a platform without epoll");
}

//******************************************************************************

void TestEpollServer::testUnsupportedEventClassification() {
   TEST_CASE("testUnsupportedEventClassification");

   PthreadsMutex fdMutex("fdMutex");
   PthreadsMutex hwmMutex("hwmMutex");
   EpollServer server(fdMutex, hwmMutex);

   requireFalse(server.isEventRead(0), "isEventRead should be false on a platform without epoll");
   requireFalse(server.isEventDisconnect(0), "isEventDisconnect should be false on a platform without epoll");
   requireFalse(server.isEventReadClose(0), "isEventReadClose should be false on a platform without epoll");
}

//******************************************************************************
