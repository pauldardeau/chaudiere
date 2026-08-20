// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestStdThreadingFactory.h"
#include "StdThreadingFactory.h"
#include "Mutex.h"
#include "Thread.h"
#include "ConditionVariable.h"
#include "ThreadPoolDispatcher.h"
#include "Runnable.h"

using namespace chaudiere;

namespace {

class DoNothingRunnable : public chaudiere::Runnable {
public:
   void run() override {
   }
};

}

//******************************************************************************

TestStdThreadingFactory::TestStdThreadingFactory() :
   poivre::TestSuite("TestStdThreadingFactory") {
}

//******************************************************************************

void TestStdThreadingFactory::runTests() {
   testCreateMutex();
   testCreateThreadWithName();
   testCreateThreadWithRunnable();
   testCreateConditionVariable();
   testCreateThreadPoolDispatcher();
}

//******************************************************************************

void TestStdThreadingFactory::testCreateMutex() {
   TEST_CASE("testCreateMutex");

   StdThreadingFactory factory;
   Mutex* mutex = factory.createMutex("myMutex");
   require(nullptr != mutex, "createMutex should return a non-null Mutex");
   require(mutex->haveValidMutex(), "created mutex should be valid");
   requireStringEquals("myMutex", mutex->getName(), "created mutex should have the given name");
   delete mutex;
}

//******************************************************************************

void TestStdThreadingFactory::testCreateThreadWithName() {
   TEST_CASE("testCreateThreadWithName");

   StdThreadingFactory factory;
   Thread* thread = factory.createThread("myThread");
   require(nullptr != thread, "createThread(name) should return a non-null Thread");
   require(nullptr == thread->getRunnable(), "a thread created without a runnable should have none");
   delete thread;
}

//******************************************************************************

void TestStdThreadingFactory::testCreateThreadWithRunnable() {
   TEST_CASE("testCreateThreadWithRunnable");

   StdThreadingFactory factory;
   DoNothingRunnable runnable;
   Thread* thread = factory.createThread(&runnable, "myThread");
   require(nullptr != thread, "createThread(runnable, name) should return a non-null Thread");
   require(&runnable == thread->getRunnable(), "the created thread should hold the given runnable");
   delete thread;
}

//******************************************************************************

void TestStdThreadingFactory::testCreateConditionVariable() {
   TEST_CASE("testCreateConditionVariable");

   StdThreadingFactory factory;
   ConditionVariable* cv = factory.createConditionVariable("myCondVar");
   require(nullptr != cv, "createConditionVariable should return a non-null ConditionVariable");
   delete cv;
}

//******************************************************************************

void TestStdThreadingFactory::testCreateThreadPoolDispatcher() {
   TEST_CASE("testCreateThreadPoolDispatcher");

   StdThreadingFactory factory;
   ThreadPoolDispatcher* dispatcher = factory.createThreadPoolDispatcher(2, "myPool");
   require(nullptr != dispatcher, "createThreadPoolDispatcher should return a non-null ThreadPoolDispatcher");
   delete dispatcher;
}

//******************************************************************************
