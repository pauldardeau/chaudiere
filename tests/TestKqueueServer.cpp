// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestKqueueServer.h"
#include "KqueueServer.h"

//******************************************************************************

TestKqueueServer::TestKqueueServer() :
   TestSuite("TestKqueueServer")
{
}

//******************************************************************************

void TestKqueueServer::runTests()
{
   testConstructor();
   testInit();
   testRun();
   testNotifySocketComplete();
}

//******************************************************************************

void TestKqueueServer::testConstructor()
{
   TEST_CASE("testConstructor");
   
}

//******************************************************************************

void TestKqueueServer::testInit()
{
   TEST_CASE("testInit");
   
}

//******************************************************************************

void TestKqueueServer::testRun()
{
   TEST_CASE("testRun");
   
}

//******************************************************************************

void TestKqueueServer::testNotifySocketComplete()
{
   TEST_CASE("testNotifySocketComplete");
   
}

//******************************************************************************
