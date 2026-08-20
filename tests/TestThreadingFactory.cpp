// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestThreadingFactory.h"
#include "ThreadingFactory.h"
#include "PthreadsThreadingFactory.h"

using namespace chaudiere;

//******************************************************************************

TestThreadingFactory::TestThreadingFactory() :
   poivre::TestSuite("TestThreadingFactory") {
}

//******************************************************************************

void TestThreadingFactory::runTests() {
   testGetThreadingFactory();
   testSetThreadingFactory();
}

//******************************************************************************

void TestThreadingFactory::testGetThreadingFactory() {
   TEST_CASE("testGetThreadingFactory");

   // a default (PthreadsThreadingFactory) instance is installed via
   // static initialization, so this should never be null
   require(nullptr != ThreadingFactory::getThreadingFactory(), "getThreadingFactory should return a non-null default instance");
}

//******************************************************************************

void TestThreadingFactory::testSetThreadingFactory() {
   TEST_CASE("testSetThreadingFactory");

   ThreadingFactory* original = ThreadingFactory::getThreadingFactory();

   PthreadsThreadingFactory replacement;
   ThreadingFactory::setThreadingFactory(&replacement);
   require(&replacement == ThreadingFactory::getThreadingFactory(), "getThreadingFactory should return the instance passed to setThreadingFactory");

   // restore the original instance so later tests/suites in this process
   // aren't affected by a stack-local factory that's about to go out of scope
   ThreadingFactory::setThreadingFactory(original);
   require(original == ThreadingFactory::getThreadingFactory(), "getThreadingFactory should return the restored original instance");
}

//******************************************************************************
