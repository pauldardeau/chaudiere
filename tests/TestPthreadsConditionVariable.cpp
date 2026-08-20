// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <atomic>

#include "TestPthreadsConditionVariable.h"
#include "PthreadsConditionVariable.h"
#include "PthreadsMutex.h"
#include "PthreadsThread.h"
#include "StdMutex.h"
#include "Runnable.h"
#include "Thread.h"

using namespace chaudiere;

namespace {

struct SharedState {
   PthreadsMutex mutex;
   PthreadsConditionVariable cv;
   bool ready;
   bool notified;

   SharedState() : mutex("waitMutex"), ready(false), notified(false) {}
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
   PthreadsMutex mutex;
   PthreadsConditionVariable cv;
   bool ready;
   std::atomic<int> notifiedCount;

   SharedStateAll() : mutex("waitMutexAll"), ready(false), notifiedCount(0) {}
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

TestPthreadsConditionVariable::TestPthreadsConditionVariable() :
   poivre::TestSuite("TestPthreadsConditionVariable") {
}

//******************************************************************************

void TestPthreadsConditionVariable::runTests() {
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

void TestPthreadsConditionVariable::testConstructor() {
   TEST_CASE("testConstructor");

   PthreadsConditionVariable cv;
   require(cv.getName().empty(), "default-constructed condition variable should have an empty name");
}

//******************************************************************************

void TestPthreadsConditionVariable::testConstructorWithName() {
   TEST_CASE("testConstructorWithName");

   PthreadsConditionVariable cv("myCondVar");
   requireStringEquals("myCondVar", cv.getName(), "name should match constructor argument");
}

//******************************************************************************

void TestPthreadsConditionVariable::testGetName() {
   TEST_CASE("testGetName");

   PthreadsConditionVariable cv("anotherName");
   requireStringEquals("anotherName", cv.getName(), "getName should return the constructor-specified name");
}

//******************************************************************************

void TestPthreadsConditionVariable::testWaitWithNullMutex() {
   TEST_CASE("testWaitWithNullMutex");

   PthreadsConditionVariable cv;
   requireFalse(cv.wait(nullptr), "wait with a null mutex should fail");
}

//******************************************************************************

void TestPthreadsConditionVariable::testWaitWithWrongMutexType() {
   TEST_CASE("testWaitWithWrongMutexType");

   PthreadsConditionVariable cv;
   StdMutex wrongTypeMutex;
   requireFalse(cv.wait(&wrongTypeMutex), "wait should fail when given a mutex that isn't a PthreadsMutex");
}

//******************************************************************************

void TestPthreadsConditionVariable::testNotifyOneNoWaiters() {
   TEST_CASE("testNotifyOneNoWaiters");

   PthreadsConditionVariable cv;
   cv.notifyOne();
   require(true, "notifyOne with no waiters should not throw");
}

//******************************************************************************

void TestPthreadsConditionVariable::testWait() {
   TEST_CASE("testWait");

   SharedState state;
   WaiterRunnable runnable(state);
   PthreadsThread thread(&runnable);
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

void TestPthreadsConditionVariable::testNotifyAll() {
   TEST_CASE("testNotifyAll");

   SharedStateAll state;
   WaiterAllRunnable r1(state);
   WaiterAllRunnable r2(state);
   WaiterAllRunnable r3(state);
   PthreadsThread t1(&r1);
   PthreadsThread t2(&r2);
   PthreadsThread t3(&r3);

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
