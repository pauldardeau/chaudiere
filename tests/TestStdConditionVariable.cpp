// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <atomic>

#include "TestStdConditionVariable.h"
#include "StdConditionVariable.h"
#include "StdMutex.h"
#include "StdThread.h"
#include "PthreadsMutex.h"
#include "Runnable.h"
#include "Thread.h"

using namespace chaudiere;

namespace {

struct SharedState {
   StdMutex mutex;
   StdConditionVariable cv;
   bool ready;
   bool notified;

   SharedState() : ready(false), notified(false) {}
};

class WaiterRunnable : public chaudiere::Runnable {
public:
   explicit WaiterRunnable(SharedState& state) : m_state(state) {}

   void run() override {
      m_state.mutex.lock();
      while (!m_state.ready) {
         m_state.cv.wait(&m_state.mutex);
      }
      m_state.notified = true;
      m_state.mutex.unlock();
   }

private:
   SharedState& m_state;
};

struct SharedStateAll {
   StdMutex mutex;
   StdConditionVariable cv;
   bool ready;
   std::atomic<int> notifiedCount;

   SharedStateAll() : ready(false), notifiedCount(0) {}
};

class WaiterAllRunnable : public chaudiere::Runnable {
public:
   explicit WaiterAllRunnable(SharedStateAll& state) : m_state(state) {}

   void run() override {
      m_state.mutex.lock();
      while (!m_state.ready) {
         m_state.cv.wait(&m_state.mutex);
      }
      ++m_state.notifiedCount;
      m_state.mutex.unlock();
   }

private:
   SharedStateAll& m_state;
};

}

//******************************************************************************

TestStdConditionVariable::TestStdConditionVariable() :
   poivre::TestSuite("TestStdConditionVariable") {
}

//******************************************************************************

void TestStdConditionVariable::runTests() {
   testConstructor();
   testConstructorWithName();
   testGetName();
   testWaitWithNullMutex();
   testWaitWithWrongMutexType();
   testNotifyOneNoWaiters();
   testWait();
   testNotifyAll();
}

//******************************************************************************

void TestStdConditionVariable::testConstructor() {
   TEST_CASE("testConstructor");

   StdConditionVariable cv;
   require(cv.getName().empty(), "default-constructed condition variable should have an empty name");
}

//******************************************************************************

void TestStdConditionVariable::testConstructorWithName() {
   TEST_CASE("testConstructorWithName");

   StdConditionVariable cv(std::string("myCondVar"));
   requireStringEquals("myCondVar", cv.getName(), "name should match constructor argument");
}

//******************************************************************************

void TestStdConditionVariable::testGetName() {
   TEST_CASE("testGetName");

   StdConditionVariable cv(std::string("anotherName"));
   requireStringEquals("anotherName", cv.getName(), "getName should return the constructor-specified name");
}

//******************************************************************************

void TestStdConditionVariable::testWaitWithNullMutex() {
   TEST_CASE("testWaitWithNullMutex");

   StdConditionVariable cv;
   requireFalse(cv.wait(nullptr), "wait with a null mutex should fail");
}

//******************************************************************************

void TestStdConditionVariable::testWaitWithWrongMutexType() {
   TEST_CASE("testWaitWithWrongMutexType");

   StdConditionVariable cv;
   PthreadsMutex wrongTypeMutex;
   requireFalse(cv.wait(&wrongTypeMutex), "wait should fail when given a mutex that isn't a StdMutex");
}

//******************************************************************************

void TestStdConditionVariable::testNotifyOneNoWaiters() {
   TEST_CASE("testNotifyOneNoWaiters");

   StdConditionVariable cv;
   cv.notifyOne();
   require(true, "notifyOne with no waiters should not throw");
}

//******************************************************************************

void TestStdConditionVariable::testWait() {
   TEST_CASE("testWait");

   SharedState state;
   WaiterRunnable runnable(state);
   StdThread thread(&runnable);
   require(thread.start(), "starting the waiter thread should succeed");

   // give the waiter thread a moment to actually reach cv.wait()
   Thread::sleep(50);

   state.mutex.lock();
   state.ready = true;
   state.mutex.unlock();
   state.cv.notifyOne();

   thread.join();
   require(state.notified, "the waiting thread should have been woken up and observed ready==true");
}

//******************************************************************************

void TestStdConditionVariable::testNotifyAll() {
   TEST_CASE("testNotifyAll");

   SharedStateAll state;
   WaiterAllRunnable r1(state);
   WaiterAllRunnable r2(state);
   WaiterAllRunnable r3(state);
   StdThread t1(&r1);
   StdThread t2(&r2);
   StdThread t3(&r3);

   require(t1.start() && t2.start() && t3.start(), "starting all three waiter threads should succeed");

   Thread::sleep(50);

   state.mutex.lock();
   state.ready = true;
   state.mutex.unlock();
   state.cv.notifyAll();

   t1.join();
   t2.join();
   t3.join();

   require(3 == state.notifiedCount.load(), "all three waiting threads should have been woken up by notifyAll");
}

//******************************************************************************
