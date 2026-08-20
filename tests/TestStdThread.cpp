// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <mutex>

#include "TestStdThread.h"
#include "StdThread.h"
#include "Thread.h"
#include "Runnable.h"

using namespace chaudiere;

namespace {

class FlagSettingRunnable : public chaudiere::Runnable {
public:
   FlagSettingRunnable() : ranCount(0) {}

   void run() override {
      ++ranCount;
   }

   int ranCount;
};

class BlockingRunnable : public chaudiere::Runnable {
public:
   BlockingRunnable() : m_started(false), m_release(false) {}

   void run() override {
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_started = true;
      }

      while (true) {
         {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_release) {
               break;
            }
         }
         Thread::sleep(5);
      }
   }

   void release() {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_release = true;
   }

   bool hasStarted() {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_started;
   }

private:
   std::mutex m_mutex;
   bool m_started;
   bool m_release;
};

}

//******************************************************************************

TestStdThread::TestStdThread() :
   poivre::TestSuite("TestStdThread") {
}

//******************************************************************************

void TestStdThread::runTests() {
   testDefaultConstructor();
   testConstructorWithName();
   testConstructorWithRunnable();
   testConstructorWithRunnableAndName();
   testGetName();
   testStartAndJoin();
   testIsAliveDuringRun();
   testGetHandle();
   testGetStdThreadId();
   testJoinWithoutStart();
}

//******************************************************************************

void TestStdThread::testDefaultConstructor() {
   TEST_CASE("testDefaultConstructor");

   StdThread thread;
   require(nullptr == thread.getRunnable(), "default-constructed thread should have no runnable");
   require(thread.getName().empty(), "default-constructed thread should have an empty name");
}

//******************************************************************************

void TestStdThread::testConstructorWithName() {
   TEST_CASE("testConstructorWithName");

   StdThread thread(std::string("myThread"));
   requireStringEquals("myThread", thread.getName(), "name should match constructor argument");
   require(nullptr == thread.getRunnable(), "a thread constructed with only a name should have no runnable");
}

//******************************************************************************

void TestStdThread::testConstructorWithRunnable() {
   TEST_CASE("testConstructorWithRunnable");

   FlagSettingRunnable runnable;
   StdThread thread(&runnable);
   require(&runnable == thread.getRunnable(), "thread should hold the runnable passed to its constructor");
}

//******************************************************************************

void TestStdThread::testConstructorWithRunnableAndName() {
   TEST_CASE("testConstructorWithRunnableAndName");

   FlagSettingRunnable runnable;
   StdThread thread(&runnable, "namedThread");
   require(&runnable == thread.getRunnable(), "thread should hold the runnable passed to its constructor");
   requireStringEquals("namedThread", thread.getName(), "name should match constructor argument");
}

//******************************************************************************

void TestStdThread::testGetName() {
   TEST_CASE("testGetName");

   StdThread thread(std::string("aName"));
   requireStringEquals("aName", thread.getName(), "getName should return the constructor-specified name");
}

//******************************************************************************

void TestStdThread::testStartAndJoin() {
   TEST_CASE("testStartAndJoin");

   FlagSettingRunnable runnable;
   StdThread thread(&runnable);
   require(thread.start(), "start() should succeed");
   thread.join();
   require(1 == runnable.ranCount, "the thread should invoke the runnable's run() exactly once");
}

//******************************************************************************

void TestStdThread::testIsAliveDuringRun() {
   TEST_CASE("testIsAliveDuringRun");

   BlockingRunnable runnable;
   StdThread thread(&runnable);

   requireFalse(thread.isAlive(), "a thread that hasn't been started should not be alive");

   thread.start();

   while (!runnable.hasStarted()) {
      Thread::sleep(5);
   }

   require(thread.isAlive(), "a thread whose runnable is still executing should be alive");

   runnable.release();
   thread.join();

   requireFalse(thread.isAlive(), "a thread should no longer be alive once its runnable has finished");
}

//******************************************************************************

void TestStdThread::testGetHandle() {
   TEST_CASE("testGetHandle");

   FlagSettingRunnable runnable;
   StdThread thread(&runnable);
   thread.start();
   thread.join();

   require(true, "getHandle should not throw once the thread has been started");
   (void) thread.getHandle();
}

//******************************************************************************

void TestStdThread::testGetStdThreadId() {
   TEST_CASE("testGetStdThreadId");

   FlagSettingRunnable runnable;
   StdThread thread(&runnable);

   const std::thread::id beforeStart = thread.getStdThreadId();
   thread.start();
   const std::thread::id afterStart = thread.getStdThreadId();
   thread.join();

   requireFalse(beforeStart == afterStart, "getStdThreadId should return a different (assigned) id once the thread has been started");
}

//******************************************************************************

void TestStdThread::testJoinWithoutStart() {
   TEST_CASE("testJoinWithoutStart");

   StdThread thread;
   thread.join();
   require(true, "join() on a thread that was never started should not throw (not joinable, so it's a no-op)");
}

//******************************************************************************
