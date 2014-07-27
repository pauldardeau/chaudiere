// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestSocketRequest.h"
#include "SocketRequest.h"

//******************************************************************************

TestSocketRequest::TestSocketRequest() :
   TestSuite("TestSocketRequest")
{
}

//******************************************************************************

void TestSocketRequest::runTests()
{
   testConstructor();
   testRun();
   testGetSocketFD();
   testGetSocket();
   testRequestComplete();
}

//******************************************************************************

void TestSocketRequest::testConstructor()
{
   TEST_CASE("testConstructor");
}

//******************************************************************************

void TestSocketRequest::testRun()
{
   TEST_CASE("testRun");
}

//******************************************************************************

void TestSocketRequest::testGetSocketFD()
{
   TEST_CASE("testGetSocketFD");
}

//******************************************************************************

void TestSocketRequest::testGetSocket()
{
   TEST_CASE("testGetSocket");
}

//******************************************************************************

void TestSocketRequest::testRequestComplete()
{
   TEST_CASE("testRequestComplete");
}

//******************************************************************************

