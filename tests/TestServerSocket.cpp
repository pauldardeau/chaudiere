// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestServerSocket.h"
#include "ServerSocket.h"

//******************************************************************************

TestServerSocket::TestServerSocket() :
   TestSuite("TestServerSocket")
{
}

//******************************************************************************

void TestServerSocket::runTests()
{
   testConstructor();
   testAccept();
   testClose();
}

//******************************************************************************

void TestServerSocket::testConstructor()
{
   TEST_CASE("testConstructor");
}

//******************************************************************************

void TestServerSocket::testAccept()
{
   TEST_CASE("testAccept");
}

//******************************************************************************

void TestServerSocket::testClose()
{
   TEST_CASE("testClose");
}

//******************************************************************************

