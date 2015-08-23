// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestThreadPool.h"
#include "ThreadPool.h"

using namespace chaudiere;

//******************************************************************************

TestThreadPool::TestThreadPool() :
   TestSuite("TestThreadPool") {
}

//******************************************************************************

void TestThreadPool::runTests() {
   testConstructor();
   testStart();
   testStop();
   testAddRequest();
   testGetNumberWorkers();
   testAddWorkers();
   testRemoveWorkers();
}

//******************************************************************************

void TestThreadPool::testConstructor() {
   TEST_CASE("testConstructor");
   //TODO: implement testConstructor
}

//******************************************************************************

void TestThreadPool::testStart() {
   TEST_CASE("testStart");
   //TODO: implement testStart
}

//******************************************************************************

void TestThreadPool::testStop() {
   TEST_CASE("testStop");
   //TODO: implement testStop
}

//******************************************************************************

void TestThreadPool::testAddRequest() {
   TEST_CASE("testAddRequest");
   //TODO: implement testAddRequest
}

//******************************************************************************

void TestThreadPool::testGetNumberWorkers() {
   TEST_CASE("testGetNumberWorkers");
   //TODO: implement testGetNumberWorkers
}

//******************************************************************************

void TestThreadPool::testAddWorkers() {
   TEST_CASE("testAddWorkers");
   //TODO: implement testAddWorkers
}

//******************************************************************************

void TestThreadPool::testRemoveWorkers() {
   TEST_CASE("testRemoveWorkers");
   //TODO: implement testRemoveWorkers
}

//******************************************************************************
