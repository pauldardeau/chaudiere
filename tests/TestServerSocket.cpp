// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestServerSocket.h"
#include "ServerSocket.h"

using namespace chaudiere;

//******************************************************************************

TestServerSocket::TestServerSocket() :
   TestSuite("TestServerSocket") {
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
   //TODO: implement testConstructor
}

//******************************************************************************

void TestServerSocket::testAccept() {
   TEST_CASE("testAccept");
   //TODO: implement testAccept
}

//******************************************************************************

void TestServerSocket::testClose() {
   TEST_CASE("testClose");
   //TODO: implement testClose
}

//******************************************************************************

