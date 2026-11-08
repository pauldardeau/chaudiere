// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSOCKETSENDINTERRUPT_H
#define CHAUDIERE_TESTSOCKETSENDINTERRUPT_H

#include "TestSuite.h"

namespace chaudiere
{

// Base fixture for TestSocketSendInterrupt's test cases (see
// TestSocketSendInterrupt.cpp), which are POIVRE_TEST_CASE self-registering test
// cases rather than hand-listed in a runTests() override - see POIVRE_TEST_CASE
// in poivre's TestMacros.h and TestThreadPool.h. Each test case gets its own
// freshly constructed instance, so this class carries no state of its own.
class TestSocketSendInterrupt : public poivre::TestSuite
{
public:
   TestSocketSendInterrupt();
};

}

#endif
