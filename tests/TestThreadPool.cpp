// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestThreadPool.h"
#include "ThreadPool.h"

//******************************************************************************

TestThreadPool::TestThreadPool() :
   TestSuite("TestThreadPool")
{
}

//******************************************************************************

void TestThreadPool::runTests()
{
   testConstructor();
   testStart();
   testStop();
   testAddRequest();
   testGetNumberWorkers();
   testAddWorkers();
   testRemoveWorkers();
}

//******************************************************************************

void TestThreadPool::testConstructor()
{
   TEST_CASE("testConstructor");
}

//******************************************************************************

void TestThreadPool::testStart()
{
   TEST_CASE("testStart");
   
}

//******************************************************************************

void TestThreadPool::testStop()
{
   TEST_CASE("testStop");
   
}

//******************************************************************************

void TestThreadPool::testAddRequest()
{
   TEST_CASE("testAddRequest");
   
}

//******************************************************************************

void TestThreadPool::testGetNumberWorkers()
{
   TEST_CASE("testGetNumberWorkers");
   
}

//******************************************************************************

void TestThreadPool::testAddWorkers()
{
   TEST_CASE("testAddWorkers");
   
}

//******************************************************************************

void TestThreadPool::testRemoveWorkers()
{
   TEST_CASE("testRemoveWorkers");
   
}

//******************************************************************************
