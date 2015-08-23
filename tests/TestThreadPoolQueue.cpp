// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestThreadPoolQueue.h"
#include "ThreadPoolQueue.h"

using namespace chaudiere;

//******************************************************************************

TestThreadPoolQueue::TestThreadPoolQueue() :
   TestSuite("TestThreadPoolQueue") {
}

//******************************************************************************

void TestThreadPoolQueue::runTests() {
   testConstructor();
   testAddRequest();
   testTakeRequest();
   testShutDown();
   testIsRunning();
   testIsEmpty();
}

//******************************************************************************

void TestThreadPoolQueue::testConstructor() {
   TEST_CASE("testConstructor");
   //TODO: implement testConstructor
}

//******************************************************************************

void TestThreadPoolQueue::testAddRequest() {
   TEST_CASE("testAddRequest");
   //TODO: implement testAddRequest
}

//******************************************************************************

void TestThreadPoolQueue::testTakeRequest() {
   TEST_CASE("testTakeRequest");
   //TODO: testTakeRequest
}

//******************************************************************************

void TestThreadPoolQueue::testShutDown() {
   TEST_CASE("testShutDown");
   //TODO: testShutDown
}

//******************************************************************************

void TestThreadPoolQueue::testIsRunning() {
   TEST_CASE("testIsRunning");
   //TODO: testIsRunning
}

//******************************************************************************

void TestThreadPoolQueue::testIsEmpty() {
   TEST_CASE("testIsEmpty");
   //TODO: implement testIsEmpty
}

//******************************************************************************

