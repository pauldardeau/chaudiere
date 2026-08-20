// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestKqueueServer.h"
#include "KqueueServer.h"
#include "PthreadsMutex.h"
#include "Socket.h"

using namespace chaudiere;

//******************************************************************************

TestKqueueServer::TestKqueueServer() :
   poivre::TestSuite("TestKqueueServer") {
}

//******************************************************************************

void TestKqueueServer::runTests() {
   testIsSupportedPlatform();
   testConstructor();
   testInit();
   testRun();
   testNotifySocketComplete();
   testUnsupportedPlatformBehavior();
}

//******************************************************************************

void TestKqueueServer::testIsSupportedPlatform() {
   TEST_CASE("testIsSupportedPlatform");

   // kqueue is only available on BSD/macOS; this test suite runs on Linux
   requireFalse(KqueueServer::isSupportedPlatform(), "kqueue should not be reported as supported on this platform");
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
   // no-op/failure value on a platform without kqueue support
   require(0 == server.getKernelEvents(10), "getKernelEvents should return 0 without kqueue support");
   require(-1 == server.fileDescriptorForEventIndex(0), "fileDescriptorForEventIndex should return -1 without kqueue support");
   requireFalse(server.addFileDescriptorForRead(0), "addFileDescriptorForRead should fail without kqueue support");
   requireFalse(server.removeFileDescriptorFromRead(0), "removeFileDescriptorFromRead should fail without kqueue support");
   requireFalse(server.isEventDisconnect(0), "isEventDisconnect should return false without kqueue support");
   requireFalse(server.isEventReadClose(0), "isEventReadClose should return false without kqueue support");
   requireFalse(server.isEventRead(0), "isEventRead should return false without kqueue support");
}

//******************************************************************************
