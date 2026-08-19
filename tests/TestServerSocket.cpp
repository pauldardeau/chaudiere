// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestServerSocket.h"
#include "ServerSocket.h"
#include "Socket.h"
#include "BasicException.h"

using namespace chaudiere;

//******************************************************************************

TestServerSocket::TestServerSocket() :
   poivre::TestSuite("TestServerSocket") {
}

//******************************************************************************

void TestServerSocket::runTests() {
   testConstructor();
   testAccept();
   testClose();
}

//******************************************************************************

void TestServerSocket::testConstructor() {
   TEST_CASE("testConstructor");

   const int port = 44700;
   ServerSocket serverSocket(port);
   require(true, "constructing a ServerSocket on an available port should succeed");

   // binding a second server socket to the same port while the first one
   // is still listening should fail and throw
   bool caughtException = false;
   try {
      ServerSocket duplicateSocket(port);
      (void) duplicateSocket;
   } catch (const BasicException&) {
      caughtException = true;
   }
   require(caughtException, "constructing a second ServerSocket on an already-bound port should throw");
}

//******************************************************************************

void TestServerSocket::testAccept() {
   TEST_CASE("testAccept");

   const int port = 44701;
   ServerSocket serverSocket(port);

   Socket clientSocket("127.0.0.1", port);
   require(clientSocket.isConnected(), "client socket should connect to the listening ServerSocket");

   Socket* acceptedSocket = serverSocket.accept();
   require(nullptr != acceptedSocket, "accept should return a non-null socket for a pending connection");
   require(acceptedSocket->isConnected(), "the accepted socket should be connected");

   delete acceptedSocket;
}

//******************************************************************************

void TestServerSocket::testClose() {
   TEST_CASE("testClose");

   const int port = 44702;
   ServerSocket serverSocket(port);

   serverSocket.close();

   // close() should be safe to call more than once
   serverSocket.close();
   require(true, "calling close() multiple times should not crash");

   // once closed, a new ServerSocket should be able to bind to the same
   // port again immediately, proving the file descriptor was released
   ServerSocket reboundSocket(port);
   require(true, "a new ServerSocket should be able to rebind the same port after close()");
}

//******************************************************************************
