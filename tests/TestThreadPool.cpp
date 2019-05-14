// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <unistd.h>

#include "TestThreadPool.h"
#include "ThreadPool.h"
#include "PthreadsThreadingFactory.h"
#include "Runnable.h"


class DoNothingRunnable : public chaudiere::Runnable
{
   public:
      virtual void run() {
      }
};

using namespace chaudiere;


//******************************************************************************

TestThreadPool::TestThreadPool() :
   TestSuite("TestThreadPool") {
}

//******************************************************************************

void TestThreadPool::runTests() {
   testConstructor();
   testStart();
   testStop();
   testAddRequest();
   testGetNumberWorkers();
   testAddWorkers();
   testRemoveWorkers();
   testGetName();
   testIsRunning();
}

//******************************************************************************

void TestThreadPool::testConstructor() {
   TEST_CASE("testConstructor");

   // number workers only
   {
      int numWorkers = 5;
      printf("%d\n", numWorkers);
      ThreadPool tp(numWorkers);

      numWorkers = 0;
      printf("%d\n", numWorkers);
      ThreadPool tpZero(numWorkers);

      numWorkers = -3;
      printf("%d\n", numWorkers);
      ThreadPool tpNeg(numWorkers);

      numWorkers = 50;
      printf("%d\n", numWorkers);
      ThreadPool tpLarge(numWorkers);
   }

   // number workers and name
   {
      const std::string name = "test_pool";

      int numWorkers = 5;
      printf("%d, %s\n", numWorkers, name.c_str());
      ThreadPool tp(numWorkers, name);

      numWorkers = 0;
      printf("%d, %s\n", numWorkers, name.c_str());
      ThreadPool tpZero(numWorkers, name);

      numWorkers = -3;
      printf("%d, %s\n", numWorkers, name.c_str());
      ThreadPool tpNeg(numWorkers, name);

      numWorkers = 50;
      printf("%d, %s\n", numWorkers, name.c_str());
      ThreadPool tpLarge(numWorkers, name);
   }

   // threading factory
   {
      ThreadingFactory* tf = new PthreadsThreadingFactory;

      int numWorkers = 5;
      printf("TF %d\n", numWorkers);
      ThreadPool tp(tf, numWorkers);

      numWorkers = 0;
      printf("TF %d\n", numWorkers);
      ThreadPool tpZero(tf, numWorkers);

      numWorkers = -3;
      printf("TF %d\n", numWorkers);
      ThreadPool tpNeg(tf, numWorkers);

      numWorkers = 50;
      printf("TF %d\n", numWorkers);
      ThreadPool tpLarge(tf, numWorkers);

      delete tf;
   }

   // threading factory and name
   {
      ThreadingFactory* tf = new PthreadsThreadingFactory;
      const std::string pool_name = "test_factory_pool";

      int numWorkers = 5;
      printf("TF %d, %s\n", numWorkers, pool_name.c_str());
      ThreadPool tp(tf, numWorkers, pool_name);

      numWorkers = 0;
      printf("TF %d, %s\n", numWorkers, pool_name.c_str());
      ThreadPool tpZero(tf, numWorkers, pool_name);

      numWorkers = -3;
      printf("TF %d, %s\n", numWorkers, pool_name.c_str());
      ThreadPool tpNeg(tf, numWorkers, pool_name);

      numWorkers = 50;
      printf("TF %d, %s\n", numWorkers, pool_name.c_str());
      ThreadPool tpLarge(tf, numWorkers, pool_name);

      delete tf;
   }
}

//******************************************************************************

void TestThreadPool::testStart() {
   TEST_CASE("testStart");

   ThreadPool tp(4);
   require(tp.stop(), "stop should succeed");
   require(tp.start(), "start should succeed");
   require(tp.isRunning(), "isRunning true after starting");
   require(!tp.start(), "start while running should fail");
   tp.stop();
}

//******************************************************************************

void TestThreadPool::testStop() {
   TEST_CASE("testStop");

   ThreadPool tp(4);
   require(tp.stop(), "stop should succeed on new pool");
   require(tp.start(), "start should succeed");
   require(tp.isRunning(), "isRunning true after starting");
   require(tp.stop(), "stop should succeed");
   require(!tp.stop(), "stop should fail after previous stop");
}

//******************************************************************************

void TestThreadPool::testAddRequest() {
   TEST_CASE("testAddRequest");

   ThreadPool tp(1);
   require(!tp.addRequest(NULL), "add NULL runnable should fail");
   require(tp.addRequest(new DoNothingRunnable), "add runnable should succeed");
   sleep(1);
   tp.stop();
   require(!tp.addRequest(new DoNothingRunnable), "add request to stopped pool should fail");
}

//******************************************************************************

void TestThreadPool::testGetNumberWorkers() {
   TEST_CASE("testGetNumberWorkers");
   int numWorkers = 5;
   ThreadPool tp(numWorkers);
   require(tp.getNumberWorkers() == numWorkers, "getNumberWorkers match init value");
   tp.stop();
}

//******************************************************************************

void TestThreadPool::testAddWorkers() {
   TEST_CASE("testAddWorkers");

   int numWorkers = 2;
   int numToAdd = 3;
   ThreadPool tp(numWorkers);
   require(tp.addWorkers(numToAdd), "add more workers should succeed");
   numWorkers += numToAdd;
   require(tp.getNumberWorkers() == numWorkers, "number workers should match expected after adding");
   require(!tp.addWorkers(0), "add 0 workers should fail");
   require(!tp.addWorkers(-2), "add negative workers should fail");
   tp.stop();
}

//******************************************************************************

void TestThreadPool::testRemoveWorkers() {
   TEST_CASE("testRemoveWorkers");

   int numWorkers = 6;
   int numToRemove = 2;
   ThreadPool tp(numWorkers);
   require(tp.removeWorkers(numToRemove), "remove workers should succeed");
   numWorkers -= numToRemove;
   require(tp.getNumberWorkers() == numWorkers, "number workers should match expected after removing");
   require(!tp.removeWorkers(0), "remove 0 workers should fail");
   require(!tp.removeWorkers(-2), "remove negative workers should fail");
   tp.stop();
}

//******************************************************************************

void TestThreadPool::testGetName() {
   TEST_CASE("testGetName");

   const std::string name = "test_pool";

   int numWorkers = 5;
   printf("%d, %s\n", numWorkers, name.c_str());
   ThreadPool tp(numWorkers, name);
   require(name == tp.getName(), "getName matches init value");
}

//******************************************************************************

void TestThreadPool::testIsRunning() {
   TEST_CASE("testIsRunning");

   ThreadPool tp(4);
   require(!tp.isRunning(), "isRunning false before starting");

   tp.start();
   require(tp.isRunning(), "isRunning true after starting");

   tp.stop();
   require(!tp.isRunning(), "isRunning false after stopping");
}

//******************************************************************************

