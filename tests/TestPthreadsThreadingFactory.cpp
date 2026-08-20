// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestPthreadsThreadingFactory.h"
#include "PthreadsThreadingFactory.h"
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

TestPthreadsThreadingFactory::TestPthreadsThreadingFactory() :
   poivre::TestSuite("TestPthreadsThreadingFactory") {
}

//******************************************************************************

void TestPthreadsThreadingFactory::runTests() {
   testCreateMutex();
   testCreateThreadWithName();
   testCreateThreadWithRunnable();
   testCreateConditionVariable();
   testCreateThreadPoolDispatcher();
}

//******************************************************************************

void TestPthreadsThreadingFactory::testCreateMutex() {
   TEST_CASE("testCreateMutex");

   PthreadsThreadingFactory factory;
   Mutex* mutex = factory.createMutex("myMutex");
   require(nullptr != mutex, "createMutex should return a non-null Mutex");
   require(mutex->haveValidMutex(), "created mutex should be valid");
   requireStringEquals("myMutex", mutex->getName(), "created mutex should have the given name");
   delete mutex;
}

//******************************************************************************

void TestPthreadsThreadingFactory::testCreateThreadWithName() {
   TEST_CASE("testCreateThreadWithName");

   PthreadsThreadingFactory factory;
   Thread* thread = factory.createThread("myThread");
   require(nullptr != thread, "createThread(name) should return a non-null Thread");
   require(nullptr == thread->getRunnable(), "a thread created without a runnable should have none");
   delete thread;
}

//******************************************************************************

void TestPthreadsThreadingFactory::testCreateThreadWithRunnable() {
   TEST_CASE("testCreateThreadWithRunnable");

   PthreadsThreadingFactory factory;
   DoNothingRunnable runnable;
   Thread* thread = factory.createThread(&runnable, "myThread");
   require(nullptr != thread, "createThread(runnable, name) should return a non-null Thread");
   require(&runnable == thread->getRunnable(), "the created thread should hold the given runnable");
   delete thread;
}

//******************************************************************************

void TestPthreadsThreadingFactory::testCreateConditionVariable() {
   TEST_CASE("testCreateConditionVariable");

   PthreadsThreadingFactory factory;
   ConditionVariable* cv = factory.createConditionVariable("myCondVar");
   require(nullptr != cv, "createConditionVariable should return a non-null ConditionVariable");
   delete cv;
}

//******************************************************************************

void TestPthreadsThreadingFactory::testCreateThreadPoolDispatcher() {
   TEST_CASE("testCreateThreadPoolDispatcher");

   PthreadsThreadingFactory factory;
   ThreadPoolDispatcher* dispatcher = factory.createThreadPoolDispatcher(2, "myPool");
   require(nullptr != dispatcher, "createThreadPoolDispatcher should return a non-null ThreadPoolDispatcher");
   delete dispatcher;
}

//******************************************************************************
