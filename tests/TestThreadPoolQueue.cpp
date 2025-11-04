// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestThreadPoolQueue.h"
#include "ThreadPoolQueue.h"
#include "PthreadsThreadingFactory.h"
#include "Runnable.h"

using namespace chaudiere;

static PthreadsThreadingFactory tf;

class DoNothingRunnable : public chaudiere::Runnable
{
   public:
      virtual void run() {
      }
};

//******************************************************************************

TestThreadPoolQueue::TestThreadPoolQueue() :
   TestSuite("TestThreadPoolQueue") {
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
   Runnable* taken = tpq.takeRequest();
   require(taken == nullptr, "takeRequest should return nullptr with nothing added");
   tpq.addRequest(r);
   taken = tpq.takeRequest();
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
   Runnable* r = tpq.takeRequest();
   require(tpq.isEmpty(), "should be empty after taking last request");
}

//******************************************************************************

