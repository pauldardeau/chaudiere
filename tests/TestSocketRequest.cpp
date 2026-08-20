// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestSocketRequest.h"
#include "SocketRequest.h"
#include "SocketServiceHandler.h"
#include "SocketCompletionObserver.h"
#include "RunCompletionObserver.h"
#include "Socket.h"

using namespace chaudiere;

namespace {

class RecordingSocketServiceHandler : public chaudiere::SocketServiceHandler {
public:
   RecordingSocketServiceHandler() : servicedRequest(nullptr) {}

   void serviceSocket(chaudiere::SocketRequest* socketRequest) override {
      servicedRequest = socketRequest;
   }

   const std::string& getName() const override {
      static const std::string name = "RecordingSocketServiceHandler";
      return name;
   }

   chaudiere::SocketRequest* servicedRequest;
};

class RecordingSocketCompletionObserver : public chaudiere::SocketCompletionObserver {
public:
   RecordingSocketCompletionObserver() : notifiedSocket(nullptr) {}

   void notifySocketComplete(chaudiere::Socket* socket) override {
      notifiedSocket = socket;
   }

   chaudiere::Socket* notifiedSocket;
};

class RecordingRunCompletionObserver : public chaudiere::RunCompletionObserver {
public:
   RecordingRunCompletionObserver() : notifiedRunnable(nullptr) {}

   void notifyRunComplete(chaudiere::Runnable* runnable) override {
      notifiedRunnable = runnable;
   }

   chaudiere::Runnable* notifiedRunnable;
};

}

//******************************************************************************

TestSocketRequest::TestSocketRequest() :
   poivre::TestSuite("TestSocketRequest") {
}

//******************************************************************************

void TestSocketRequest::runTests() {
   testConstructor();
   testConstructorWithCompletionObserver();
   testRun();
   testGetSocketFD();
   testGetSocket();
   testRequestComplete();
   testIsSocketOwned();
   testSetSocketOwned();
   testNotifyOnCompletion();
}

//******************************************************************************

void TestSocketRequest::testConstructor() {
   TEST_CASE("testConstructor");

   Socket* socket = new Socket(-1);
   RecordingSocketServiceHandler handler;
   SocketRequest request(socket, &handler);

   require(socket == request.getSocket(), "getSocket should return the socket passed to the constructor");
   require(request.isSocketOwned(), "socket should be owned by default when constructed with a raw Socket*");
   // request's destructor deletes socket
}

//******************************************************************************

void TestSocketRequest::testConstructorWithCompletionObserver() {
   TEST_CASE("testConstructorWithCompletionObserver");

   RecordingSocketCompletionObserver observer;
   const int fd = Socket::createSocket();
   RecordingSocketServiceHandler handler;
   SocketRequest request(&observer, fd, &handler);

   require(nullptr != request.getSocket(), "getSocket should be non-null for the completion-observer constructor");
   require(fd == request.getSocketFD(), "getSocketFD should return the given file descriptor");
   requireFalse(request.isSocketOwned(), "socket should not be owned by default for the completion-observer constructor");
}

//******************************************************************************

void TestSocketRequest::testRun() {
   TEST_CASE("testRun");

   Socket* socket = new Socket(-1);
   RecordingSocketServiceHandler handler;
   SocketRequest request(socket, &handler);

   request.run();
   require(&request == handler.servicedRequest, "run should dispatch to the handler's serviceSocket with itself");
}

//******************************************************************************

void TestSocketRequest::testGetSocketFD() {
   TEST_CASE("testGetSocketFD");

   const int fd = Socket::createSocket();
   Socket* socket = new Socket(fd);
   RecordingSocketServiceHandler handler;
   SocketRequest request(socket, &handler);

   require(fd == request.getSocketFD(), "getSocketFD should return the underlying socket's file descriptor");
}

//******************************************************************************

void TestSocketRequest::testGetSocket() {
   TEST_CASE("testGetSocket");

   Socket* socket = new Socket(-1);
   RecordingSocketServiceHandler handler;
   SocketRequest request(socket, &handler);

   require(socket == request.getSocket(), "getSocket should return the constructor-provided socket");
}

//******************************************************************************

void TestSocketRequest::testRequestComplete() {
   TEST_CASE("testRequestComplete");

   RecordingSocketCompletionObserver observer;
   const int fd = Socket::createSocket();
   Socket* socket = new Socket(&observer, fd);
   RecordingSocketServiceHandler handler;
   SocketRequest request(socket, &handler);

   require(nullptr == observer.notifiedSocket, "observer should not be notified before requestComplete is called");
   request.requestComplete();
   require(socket == observer.notifiedSocket, "requestComplete should notify the socket's completion observer");
}

//******************************************************************************

void TestSocketRequest::testIsSocketOwned() {
   TEST_CASE("testIsSocketOwned");

   Socket* socket = new Socket(-1);
   RecordingSocketServiceHandler handler;
   SocketRequest request(socket, &handler);
   require(request.isSocketOwned(), "socket should be owned by default when constructed with a raw Socket*");
}

//******************************************************************************

void TestSocketRequest::testSetSocketOwned() {
   TEST_CASE("testSetSocketOwned");

   Socket* socket = new Socket(-1);
   RecordingSocketServiceHandler handler;
   SocketRequest request(socket, &handler);
   request.setSocketOwned(false);
   requireFalse(request.isSocketOwned(), "isSocketOwned should reflect setSocketOwned(false)");

   // NOTE: SocketRequest's destructor always deletes its borrowed socket
   // regardless of the isSocketOwned flag -- that flag is metadata read by
   // external code (e.g. SocketServer::serviceSocket(), which propagates
   // it to the RequestHandler it creates so ownership isn't claimed
   // twice), not something that gates this object's own cleanup. So no
   // manual delete here -- request's destructor still owns and deletes
   // socket.
}

//******************************************************************************

void TestSocketRequest::testNotifyOnCompletion() {
   TEST_CASE("testNotifyOnCompletion");

   RecordingSocketCompletionObserver socketObserver;
   const int fd = Socket::createSocket();
   Socket* socket = new Socket(&socketObserver, fd);
   RecordingSocketServiceHandler handler;
   SocketRequest request(socket, &handler);

   RecordingRunCompletionObserver runObserver;
   request.setCompletionObserver(&runObserver);

   request.notifyOnCompletion();

   require(socket == socketObserver.notifiedSocket, "notifyOnCompletion should call requestComplete(), notifying the socket's completion observer");
   require(&request == runObserver.notifiedRunnable, "notifyOnCompletion should also notify the registered RunCompletionObserver");
}

//******************************************************************************
