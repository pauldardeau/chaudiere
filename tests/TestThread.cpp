// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestThread.h"
#include "Thread.h"

using namespace chaudiere;

//******************************************************************************

TestThread::TestThread() :
   TestSuite("TestThread") {
}

//******************************************************************************

void TestThread::runTests() {
   testConstructor();
   testConstructorWithRunnable();
   testSetPoolWorkerStatus();
   testIsPoolWorker();
   testStart();
   testRun();
   testIsAlive();
   testGetExitCode();
   testRegisterThreadCompletionObserver();
   testClearThreadCompletionObserver();
   testGetRunnable();
   testGetHandle();
   testSetAttribute();
   testHasAttribute();
   testGetAttribute();
   testClearAttribute();
}

//******************************************************************************

void TestThread::testConstructor() {
   TEST_CASE("testConstructor");
   //TODO: implement testConstructor
}

//******************************************************************************

void TestThread::testConstructorWithRunnable() {
   TEST_CASE("testConstructorWithRunnable");
   //TODO: testConstructorWithRunnable
}

//******************************************************************************

void TestThread::testSetPoolWorkerStatus() {
   TEST_CASE("testSetPoolWorkerStatus");
   //TODO: implement testSetPoolWorkerStatus
}

//******************************************************************************

void TestThread::testIsPoolWorker() {
   TEST_CASE("testIsPoolWorker");
   //TODO: implement testIsPoolWorker
}

//******************************************************************************

void TestThread::testStart() {
   TEST_CASE("testStart");
   //TODO: implement testStart
}

//******************************************************************************

void TestThread::testRun() {
   TEST_CASE("testRun");
   //TODO: implement testRun
}

//******************************************************************************

void TestThread::testIsAlive() {
   TEST_CASE("testIsAlive");
   //TODO: implement testIsAlive
}

//******************************************************************************

void TestThread::testGetExitCode() {
   TEST_CASE("testGetExitCode");
   //TODO: implement testGetExitCode
}

//******************************************************************************

void TestThread::testRegisterThreadCompletionObserver() {
   TEST_CASE("testRegisterThreadCompletionObserver");
   //TODO: implement testRegisterThreadCompletionObserver
}

//******************************************************************************

void TestThread::testClearThreadCompletionObserver() {
   TEST_CASE("testClearThreadCompletionObserver");
   //TODO: implement testClearThreadCompletionObserver
}

//******************************************************************************

void TestThread::testGetRunnable() {
   TEST_CASE("testGetRunnable");
   //TODO: implement testGetRunnable
}

//******************************************************************************

void TestThread::testGetHandle() {
   TEST_CASE("testGetHandle");
   //TODO: implement testGetHandle
}

//******************************************************************************

void TestThread::testSetAttribute() {
   TEST_CASE("testSetAttribute");
   //TODO: implement testSetAttribute
}

//******************************************************************************

void TestThread::testHasAttribute() {
   TEST_CASE("testHasAttribute");
   //TODO: implement testHasAttribute
}

//******************************************************************************

void TestThread::testGetAttribute() {
   TEST_CASE("testGetAttribute");
   //TODO: implement testGetAttribute
}

//******************************************************************************

void TestThread::testClearAttribute() {
   TEST_CASE("testClearAttribute");
   //TODO: implement testClearAttribute
}

//******************************************************************************

