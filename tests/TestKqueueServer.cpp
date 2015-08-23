// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestKqueueServer.h"
#include "KqueueServer.h"

using namespace chaudiere;

//******************************************************************************

TestKqueueServer::TestKqueueServer() :
   TestSuite("TestKqueueServer") {
}

//******************************************************************************

void TestKqueueServer::runTests() {
   testConstructor();
   testInit();
   testRun();
   testNotifySocketComplete();
}

//******************************************************************************

void TestKqueueServer::testConstructor() {
   TEST_CASE("testConstructor");
   //TODO: implement testConstructor
}

//******************************************************************************

void TestKqueueServer::testInit() {
   TEST_CASE("testInit");
   //TODO: implement testInit
}

//******************************************************************************

void TestKqueueServer::testRun() {
   TEST_CASE("testRun");
   //TODO: implement testRun
}

//******************************************************************************

void TestKqueueServer::testNotifySocketComplete() {
   TEST_CASE("testNotifySocketComplete");
   //TODO: implement testNotifySocketComplete
}

//******************************************************************************
