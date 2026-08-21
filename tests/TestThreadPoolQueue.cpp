// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <atomic>

#include "TestThreadPoolQueue.h"
#include "ThreadPoolQueue.h"
#include "PthreadsThreadingFactory.h"
#include "PthreadsThread.h"
#include "Thread.h"
#include "Runnable.h"

using namespace chaudiere;

static PthreadsThreadingFactory tf;

class DoNothingRunnable : public chaudiere::Runnable
{
   public:
      virtual void run() {
      }

      virtual void notifyOnCompletion() {
         printf("DoNothingRunnable::notifyOnCompletion\n");
         chaudiere::Runnable::notifyOnCompletion();
      }
};

//******************************************************************************

TestThreadPoolQueue::TestThreadPoolQueue() :
   poivre::TestSuite("TestThreadPoolQueue") {
}

//******************************************************************************

void TestThreadPoolQueue::runTests() {
   testConstructor();
   testAddRequest();
   testTakeRequest();
   testShutDown();
   testIsRunning();
   testIsEmpty();
}

//******************************************************************************

void TestThreadPoolQueue::testConstructor() {
   TEST_CASE("testConstructor");

   ThreadPoolQueue tpq(&tf);
   require(tpq.isRunning(), "should be running after construction");
   require(tpq.isEmpty(), "should be empty after construction");
}

//******************************************************************************

void TestThreadPoolQueue::testAddRequest() {
   TEST_CASE("testAddRequest");

   Runnable* r = new DoNothingRunnable;
   ThreadPoolQueue tpq(&tf);
   require(tpq.addRequest(r), "addRequest should succeed");
   require(!tpq.isEmpty(), "should not be empty after adding request");
   tpq.shutDown();
   require(!tpq.addRequest(r), "addRequest should fail after shutDown");
   delete r;
}

//******************************************************************************

void TestThreadPoolQueue::testTakeRequest() {
   TEST_CASE("testTakeRequest");

   Runnable* r = new DoNothingRunnable;
   ThreadPoolQueue tpq(&tf);
   TakeRequestContext ctx;
   ctx.waitIfNone = false;
   tpq.takeRequest(ctx);
   Runnable* taken = ctx.runnable;
   require(taken == nullptr, "takeRequest should return nullptr with nothing added");
   tpq.addRequest(r);
   tpq.takeRequest(ctx);
   taken = ctx.runnable;
   require(taken != nullptr, "takeRequest should return non-nullptr after adding");
   require(taken == r, "takeRequest should return what was added");
   delete r;
}

//******************************************************************************

void TestThreadPoolQueue::testShutDown() {
   TEST_CASE("testShutDown");

   ThreadPoolQueue tpq(&tf);
   tpq.shutDown();
   require(!tpq.isRunning(), "should not be running after shutDown");
}

//******************************************************************************

void TestThreadPoolQueue::testIsRunning() {
   TEST_CASE("testIsRunning");

   /*
   ThreadPoolQueue tpq(&tf);
   require(tpq.isRunning(), "should be running after construction");
   tpq.shutDown();
   require(!tpq.isRunning(), "should not be running after shutDown");
   tpq.startUp();
   require(tpq.isRunning(), "should be running after startUp");
   */
}

//******************************************************************************

void TestThreadPoolQueue::testIsEmpty() {
   TEST_CASE("testIsEmpty");

   ThreadPoolQueue tpq(&tf);
   require(tpq.isEmpty(), "initial state should be empty");
   tpq.addRequest(new DoNothingRunnable);
   require(!tpq.isEmpty(), "should not be empty after adding a request");
   TakeRequestContext ctx;
   tpq.takeRequest(ctx);
   require(tpq.isEmpty(), "should be empty after taking last request");

   // ThreadPoolQueue doesn't own the Runnable (isAutoDelete defaults
   // to false), so it's this test's responsibility to delete it.
   delete ctx.runnable;
}

//******************************************************************************
// backpressure (setMaxQueueSize()/QueueFullPolicy) - written as
// self-registering POIVRE_TEST_CASE cases (see poivre's TestMacros.h)
// rather than added to runTests() above, since two of them need to
// spawn and synchronize with a real second thread.

namespace {

// runs queue.addRequest(toAdd) on its own thread, so a test can observe
// whether that call is currently blocked (hasCompleted() still false)
// or has returned (hasCompleted() true, getResult() has its outcome)
class BlockingAddRunnable : public chaudiere::Runnable {
public:
   BlockingAddRunnable(ThreadPoolQueue& queue, Runnable* toAdd) :
      m_queue(queue),
      m_toAdd(toAdd),
      m_completed(false),
      m_result(false) {
   }

   void run() override {
      m_result = m_queue.addRequest(m_toAdd);
      m_completed = true;
   }

   bool hasCompleted() const {
      return m_completed;
   }

   bool getResult() const {
      return m_result;
   }

private:
   ThreadPoolQueue& m_queue;
   Runnable* m_toAdd;
   std::atomic<bool> m_completed;
   std::atomic<bool> m_result;
};

}

POIVRE_TEST_CASE(TestThreadPoolQueue, testMaxQueueSizeDefaultUnbounded) {
   ThreadPoolQueue tpq(&tf);

   require(tpq.getMaxQueueSize() == 0, "max queue size should default to 0 (unbounded)");

   Runnable* r1 = new DoNothingRunnable;
   Runnable* r2 = new DoNothingRunnable;
   Runnable* r3 = new DoNothingRunnable;

   require(tpq.addRequest(r1), "unbounded queue should accept request 1");
   require(tpq.addRequest(r2), "unbounded queue should accept request 2");
   require(tpq.addRequest(r3), "unbounded queue should accept request 3");

   delete r1;
   delete r2;
   delete r3;
}

POIVRE_TEST_CASE(TestThreadPoolQueue, testMaxQueueSizeRejectPolicy) {
   ThreadPoolQueue tpq(&tf);
   tpq.setMaxQueueSize(2, QueueFullPolicy::Reject);
   require(tpq.getMaxQueueSize() == 2, "getMaxQueueSize should reflect what was set");

   Runnable* r1 = new DoNothingRunnable;
   Runnable* r2 = new DoNothingRunnable;
   Runnable* r3 = new DoNothingRunnable;

   require(tpq.addRequest(r1), "request 1 should be accepted (0 -> 1, max 2)");
   require(tpq.addRequest(r2), "request 2 should be accepted (1 -> 2, max 2)");
   require(!tpq.addRequest(r3), "request 3 should be rejected - queue already at max size 2");

   // freeing a slot should allow a subsequent add to succeed
   TakeRequestContext ctx;
   ctx.waitIfNone = false;
   tpq.takeRequest(ctx);
   require(ctx.runnable == r1, "takeRequest should return the oldest request (FIFO)");

   require(tpq.addRequest(r3), "request 3 should now be accepted after a slot freed up");

   delete r1;
   delete r2;
   delete r3;
}

POIVRE_TEST_CASE(TestThreadPoolQueue, testMaxQueueSizeBlockPolicyBlocksThenUnblocks) {
   ThreadPoolQueue tpq(&tf);
   tpq.setMaxQueueSize(1, QueueFullPolicy::Block);

   Runnable* r1 = new DoNothingRunnable;
   Runnable* r2 = new DoNothingRunnable;

   require(tpq.addRequest(r1), "request 1 should be accepted immediately (0 -> 1, max 1)");

   BlockingAddRunnable blockingAdd(tpq, r2);
   PthreadsThread blockingThread(&blockingAdd);
   require(blockingThread.start(), "helper thread should start");

   // generous margin for the helper thread to actually reach the wait
   // inside addRequest() - not a hard guarantee, but more than enough
   // in practice, and the test still holds even if it wins this race
   // (it would just also pass sooner)
   Thread::sleep(200);
   require(!blockingAdd.hasCompleted(), "addRequest for request 2 should still be blocked - queue is full");

   // free the one slot - should wake and complete the blocked add
   TakeRequestContext ctx;
   ctx.waitIfNone = false;
   tpq.takeRequest(ctx);
   require(ctx.runnable == r1, "takeRequest should return request 1");

   blockingThread.join();

   require(blockingAdd.hasCompleted(), "blocked addRequest should have completed after a slot freed up");
   require(blockingAdd.getResult(), "blocked addRequest should have succeeded once space was available");

   delete r1;
   delete r2;
}

POIVRE_TEST_CASE(TestThreadPoolQueue, testMaxQueueSizeBlockPolicyWakesOnShutdown) {
   ThreadPoolQueue tpq(&tf);
   tpq.setMaxQueueSize(1, QueueFullPolicy::Block);

   Runnable* r1 = new DoNothingRunnable;
   Runnable* r2 = new DoNothingRunnable;

   require(tpq.addRequest(r1), "request 1 should be accepted immediately");

   BlockingAddRunnable blockingAdd(tpq, r2);
   PthreadsThread blockingThread(&blockingAdd);
   require(blockingThread.start(), "helper thread should start");

   Thread::sleep(200);
   require(!blockingAdd.hasCompleted(), "addRequest for request 2 should still be blocked - queue is full");

   tpq.shutDown();
   blockingThread.join();

   require(blockingAdd.hasCompleted(), "blocked addRequest should have woken up once the queue shut down, not hung forever");
   require(!blockingAdd.getResult(), "blocked addRequest should fail once the queue has shut down");

   delete r1;
   delete r2;
}

//******************************************************************************

