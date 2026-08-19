// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestRequestHandler.h"
#include "RequestHandler.h"
#include "Socket.h"
#include "SocketRequest.h"
#include "SocketServiceHandler.h"
#include "RunCompletionObserver.h"

using namespace chaudiere;

namespace {

// RequestHandler is abstract (it inherits Runnable::run() as pure
// virtual without overriding it), so a concrete subclass is needed to
// exercise its constructors and accessors.
class ConcreteRequestHandler : public chaudiere::RequestHandler {
public:
   explicit ConcreteRequestHandler(chaudiere::Socket* socket) :
      chaudiere::RequestHandler(socket) {
   }

   explicit ConcreteRequestHandler(chaudiere::SocketRequest* socketRequest) :
      chaudiere::RequestHandler(socketRequest) {
   }

   void run() override {
   }
};

class RecordingCompletionObserver : public chaudiere::RunCompletionObserver {
public:
   RecordingCompletionObserver() : notifiedRunnable(nullptr) {}

   void notifyRunComplete(chaudiere::Runnable* runnable) override {
      notifiedRunnable = runnable;
   }

   chaudiere::Runnable* notifiedRunnable;
};

// Unused by these tests directly, but required to construct a SocketRequest.
class NoOpSocketServiceHandler : public chaudiere::SocketServiceHandler {
public:
   void serviceSocket(chaudiere::SocketRequest*) override {
   }

   const std::string& getName() const override {
      static const std::string name = "NoOpSocketServiceHandler";
      return name;
   }
};

}

//******************************************************************************

TestRequestHandler::TestRequestHandler() :
   poivre::TestSuite("TestRequestHandler") {
}

//******************************************************************************

void TestRequestHandler::runTests() {
   testConstructorWithSocket();
   testConstructorWithSocketRequest();
   testSetThreadPooling();
   testIsThreadPooling();
   testGetSocket();
   testIsSocketOwned();
   testSetSocketOwned();
   testNotifyOnCompletion();
}

//******************************************************************************

void TestRequestHandler::testConstructorWithSocket() {
   TEST_CASE("testConstructorWithSocket");

   Socket* socket = new Socket(-1);
   ConcreteRequestHandler handler(socket);

   require(handler.getSocket() == socket, "getSocket should return the socket passed to the constructor");
   require(handler.isSocketOwned(), "socket should be owned by the handler by default");

   // handler's destructor closes and deletes the owned socket
}

//******************************************************************************

void TestRequestHandler::testConstructorWithSocketRequest() {
   TEST_CASE("testConstructorWithSocketRequest");

   NoOpSocketServiceHandler serviceHandler;
   Socket* socket = new Socket(-1);
   SocketRequest* socketRequest = new SocketRequest(socket, &serviceHandler);

   {
      ConcreteRequestHandler handler(socketRequest);
      require(handler.getSocket() == socket, "getSocket should return the socket wrapped by the SocketRequest");
   }

   // RequestHandler doesn't own the SocketRequest (isAutoDelete defaults
   // to false), so it's this test's responsibility to delete it;
   // SocketRequest's destructor deletes socket in turn.
   delete socketRequest;
}

//******************************************************************************

void TestRequestHandler::testSetThreadPooling() {
   TEST_CASE("testSetThreadPooling");

   ConcreteRequestHandler handler(new Socket(-1));
   requireFalse(handler.isThreadPooling(), "threadPooling should default to false");
   handler.setThreadPooling(true);
   require(handler.isThreadPooling(), "isThreadPooling should reflect setThreadPooling(true)");
   handler.setThreadPooling(false);
   requireFalse(handler.isThreadPooling(), "isThreadPooling should reflect setThreadPooling(false)");
}

//******************************************************************************

void TestRequestHandler::testIsThreadPooling() {
   TEST_CASE("testIsThreadPooling");

   ConcreteRequestHandler handler(new Socket(-1));
   requireFalse(handler.isThreadPooling(), "isThreadPooling should be false by default");
}

//******************************************************************************

void TestRequestHandler::testGetSocket() {
   TEST_CASE("testGetSocket");

   Socket* directSocket = new Socket(-1);
   ConcreteRequestHandler directHandler(directSocket);
   require(directHandler.getSocket() == directSocket, "getSocket should return the directly-owned socket");

   NoOpSocketServiceHandler serviceHandler;
   Socket* wrappedSocket = new Socket(-1);
   SocketRequest* socketRequest = new SocketRequest(wrappedSocket, &serviceHandler);
   {
      ConcreteRequestHandler handlerFromRequest(socketRequest);
      require(handlerFromRequest.getSocket() == wrappedSocket, "getSocket should return the socket wrapped by the SocketRequest");
   }
   delete socketRequest;
}

//******************************************************************************

void TestRequestHandler::testIsSocketOwned() {
   TEST_CASE("testIsSocketOwned");

   ConcreteRequestHandler handler(new Socket(-1));
   require(handler.isSocketOwned(), "socket should be owned by default");
}

//******************************************************************************

void TestRequestHandler::testSetSocketOwned() {
   TEST_CASE("testSetSocketOwned");

   Socket* socket = new Socket(-1);
   {
      ConcreteRequestHandler handler(socket);
      handler.setSocketOwned(false);
      requireFalse(handler.isSocketOwned(), "isSocketOwned should reflect setSocketOwned(false)");
      // handler's destructor runs at the end of this scope: it still
      // calls socket->close() unconditionally, but setSocketOwned(false)
      // means it will not delete socket -- so socket must still be alive
      // when that destructor runs, and it's this test's job to delete it
      // afterward instead.
   }
   delete socket;
}

//******************************************************************************

void TestRequestHandler::testNotifyOnCompletion() {
   TEST_CASE("testNotifyOnCompletion");

   RecordingCompletionObserver observer;
   ConcreteRequestHandler handler(new Socket(-1));
   handler.setCompletionObserver(&observer);

   handler.notifyOnCompletion();

   require(&handler == observer.notifiedRunnable, "notifyOnCompletion should notify the registered RunCompletionObserver with this handler");
}

//******************************************************************************
