// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <mutex>

#include "TestThread.h"
#include "Thread.h"
#include "PthreadsThread.h"
#include "ThreadCompletionObserver.h"
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

// A Runnable that blocks until explicitly released, so tests can reliably
// observe a thread while it's actually running.
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

class RecordingCompletionObserver : public chaudiere::ThreadCompletionObserver {
public:
   RecordingCompletionObserver() : notifiedThread(nullptr) {}

   void notifyThreadComplete(chaudiere::Thread* thread) override {
      notifiedThread = thread;
   }

   chaudiere::Thread* notifiedThread;
};

}

//******************************************************************************

TestThread::TestThread() :
   poivre::TestSuite("TestThread") {
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

   PthreadsThread thread;
   require(nullptr == thread.getRunnable(), "default-constructed thread should have no runnable");
   requireFalse(thread.isAlive(), "a newly constructed thread should not be alive");
}

//******************************************************************************

void TestThread::testConstructorWithRunnable() {
   TEST_CASE("testConstructorWithRunnable");

   FlagSettingRunnable runnable;
   PthreadsThread thread(&runnable);
   require(&runnable == thread.getRunnable(), "thread should hold the runnable passed to its constructor");
}

//******************************************************************************

void TestThread::testSetPoolWorkerStatus() {
   TEST_CASE("testSetPoolWorkerStatus");

   PthreadsThread thread;
   requireFalse(thread.isPoolWorker(), "pool worker status should default to false");
   thread.setPoolWorkerStatus(true);
   require(thread.isPoolWorker(), "isPoolWorker should reflect setPoolWorkerStatus(true)");
   thread.setPoolWorkerStatus(false);
   requireFalse(thread.isPoolWorker(), "isPoolWorker should reflect setPoolWorkerStatus(false)");
}

//******************************************************************************

void TestThread::testIsPoolWorker() {
   TEST_CASE("testIsPoolWorker");

   PthreadsThread thread;
   requireFalse(thread.isPoolWorker(), "isPoolWorker should be false by default");
}

//******************************************************************************

void TestThread::testStart() {
   TEST_CASE("testStart");

   FlagSettingRunnable runnable;
   PthreadsThread thread(&runnable);
   require(thread.start(), "start() should succeed");
   thread.join();
   require(runnable.ranCount > 0, "starting the thread should have run the runnable");
}

//******************************************************************************

void TestThread::testRun() {
   TEST_CASE("testRun");

   FlagSettingRunnable runnable;
   PthreadsThread thread(&runnable);
   thread.start();
   thread.join();
   require(1 == runnable.ranCount, "the thread should invoke the runnable's run() exactly once");
}

//******************************************************************************

void TestThread::testIsAlive() {
   TEST_CASE("testIsAlive");

   BlockingRunnable runnable;
   PthreadsThread thread(&runnable);

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

void TestThread::testGetExitCode() {
   TEST_CASE("testGetExitCode");

   FlagSettingRunnable runnable;
   PthreadsThread thread(&runnable);
   thread.start();
   thread.join();
   require(0 == thread.getExitCode(), "getExitCode should be 0 after a runnable completes without throwing");
}

//******************************************************************************

void TestThread::testRegisterThreadCompletionObserver() {
   TEST_CASE("testRegisterThreadCompletionObserver");

   RecordingCompletionObserver observer;
   FlagSettingRunnable runnable;
   PthreadsThread thread(&runnable);
   thread.registerThreadCompletionObserver(&observer);

   thread.start();
   thread.join();

   // by the time join() returns, the thread routine has already called
   // setAlive(false) and notifyOnCompletion(), so the observer is
   // guaranteed to have already been notified
   require(&thread == observer.notifiedThread, "the registered observer should be notified with this thread once it completes");
}

//******************************************************************************

void TestThread::testClearThreadCompletionObserver() {
   TEST_CASE("testClearThreadCompletionObserver");

   RecordingCompletionObserver observer;
   FlagSettingRunnable runnable;
   PthreadsThread thread(&runnable);
   thread.registerThreadCompletionObserver(&observer);
   thread.clearThreadCompletionObserver();

   thread.start();
   thread.join();

   require(nullptr == observer.notifiedThread, "an observer that's been cleared should not be notified");
}

//******************************************************************************

void TestThread::testGetRunnable() {
   TEST_CASE("testGetRunnable");

   FlagSettingRunnable runnable;
   PthreadsThread thread(&runnable);
   require(&runnable == thread.getRunnable(), "getRunnable should return the runnable passed to the constructor");

   PthreadsThread noRunnableThread;
   require(nullptr == noRunnableThread.getRunnable(), "getRunnable should be null for a thread constructed without a runnable");
}

//******************************************************************************

void TestThread::testGetHandle() {
   TEST_CASE("testGetHandle");

   FlagSettingRunnable runnable;
   PthreadsThread thread(&runnable);
   const pthread_t beforeStart = thread.getHandle();

   thread.start();
   thread.join();

   const pthread_t afterStart = thread.getHandle();
   requireFalse(0 != pthread_equal(beforeStart, afterStart), "getHandle should return a different (assigned) handle once the thread has been started");
}

//******************************************************************************

void TestThread::testSetAttribute() {
   TEST_CASE("testSetAttribute");

   PthreadsThread thread;
   thread.setAttribute("key1", "value1");
   require(thread.hasAttribute("key1"), "attribute should be present after setAttribute");
   requireStringEquals("value1", thread.getAttribute("key1"), "getAttribute should return the value set");
}

//******************************************************************************

void TestThread::testHasAttribute() {
   TEST_CASE("testHasAttribute");

   PthreadsThread thread;
   requireFalse(thread.hasAttribute("missing"), "hasAttribute should be false for an attribute that was never set");
   thread.setAttribute("present", "x");
   require(thread.hasAttribute("present"), "hasAttribute should be true once the attribute has been set");
}

//******************************************************************************

void TestThread::testGetAttribute() {
   TEST_CASE("testGetAttribute");

   PthreadsThread thread;
   thread.setAttribute("greeting", "hello");
   requireStringEquals("hello", thread.getAttribute("greeting"), "getAttribute should return the value that was set");
}

//******************************************************************************

void TestThread::testClearAttribute() {
   TEST_CASE("testClearAttribute");

   PthreadsThread thread;
   thread.setAttribute("temp", "value");
   require(thread.hasAttribute("temp"), "sanity check: attribute should be present before clearing");
   thread.clearAttribute("temp");
   requireFalse(thread.hasAttribute("temp"), "hasAttribute should be false after clearAttribute");
}

//******************************************************************************
