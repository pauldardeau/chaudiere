// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTTHREADPOOL_H
#define CHAUDIERE_TESTTHREADPOOL_H

#include "TestSuite.h"

namespace chaudiere
{

// Base fixture for TestThreadPool's test cases (see TestThreadPool.cpp),
// which are POIVRE_TEST_CASE self-registering test cases rather than
// hand-listed in a runTests() override - see POIVRE_TEST_CASE in
// poivre's TestMacros.h. Each test case gets its own freshly constructed
// TestThreadPool instance, so this class carries no state of its own; it
// exists only to provide the "TestThreadPool" suite name and the
// inherited require()/requireFalse()/etc. assertions.
class TestThreadPool : public poivre::TestSuite
{
public:
   TestThreadPool();
};

}

#endif
