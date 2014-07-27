// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestThreadPoolQueue.h"
#include "ThreadPoolQueue.h"

//******************************************************************************

TestThreadPoolQueue::TestThreadPoolQueue() :
   TestSuite("TestThreadPoolQueue")
{
}

//******************************************************************************

void TestThreadPoolQueue::runTests()
{
   testConstructor();
   testAddRequest();
   testTakeRequest();
   testShutDown();
   testIsRunning();
   testIsEmpty();
}

//******************************************************************************

void TestThreadPoolQueue::testConstructor()
{
   TEST_CASE("testConstructor");
}

//******************************************************************************

void TestThreadPoolQueue::testAddRequest()
{
   TEST_CASE("testAddRequest");
}

//******************************************************************************

void TestThreadPoolQueue::testTakeRequest()
{
   TEST_CASE("testTakeRequest");
}

//******************************************************************************

void TestThreadPoolQueue::testShutDown()
{
   TEST_CASE("testShutDown");
}

//******************************************************************************

void TestThreadPoolQueue::testIsRunning()
{
   TEST_CASE("testIsRunning");
}

//******************************************************************************

void TestThreadPoolQueue::testIsEmpty()
{
   TEST_CASE("testIsEmpty");
}

//******************************************************************************

