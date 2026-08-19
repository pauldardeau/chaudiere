// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <atomic>

#include "TestThreadPoolWorker.h"
#include "ThreadPoolWorker.h"
#include "ThreadPoolQueue.h"
#include "PthreadsThreadingFactory.h"
#include "Runnable.h"
#include "Thread.h"

using namespace chaudiere;

static PthreadsThreadingFactory tf;

namespace {

class CountingRunnable : public chaudiere::Runnable {
public:
   explicit CountingRunnable(std::atomic<int>& counter) : m_counter(counter) {}

   void run() override {
      ++m_counter;
   }

private:
   std::atomic<int>& m_counter;
};

}

//******************************************************************************

TestThreadPoolWorker::TestThreadPoolWorker() :
   poivre::TestSuite("TestThreadPoolWorker") {
}

//******************************************************************************

void TestThreadPoolWorker::runTests() {
   testConstructor();
   testRun();
}

//******************************************************************************

void TestThreadPoolWorker::testConstructor() {
   TEST_CASE("testConstructor");

   ThreadPoolQueue queue(&tf);
   ThreadPoolWorker worker(&tf, queue, 1);
   require(true, "constructing a ThreadPoolWorker should not throw");
}

//******************************************************************************

void TestThreadPoolWorker::testRun() {
   TEST_CASE("testRun");

   std::atomic<int> executionCount(0);

   ThreadPoolQueue queue(&tf);
   ThreadPoolWorker worker(&tf, queue, 1);
   worker.start();

   CountingRunnable runnable(executionCount);
   queue.addRequest(&runnable);

   // give the worker thread a moment to pick up and run the request
   int waitedMsec = 0;
   while ((0 == executionCount.load()) && (waitedMsec < 2000)) {
      Thread::sleep(5);
      waitedMsec += 5;
   }

   require(1 == executionCount.load(), "the queued runnable should have been run by the worker thread exactly once");

   queue.shutDown();
   worker.stop();
}

//******************************************************************************
