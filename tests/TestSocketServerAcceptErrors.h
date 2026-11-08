// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSOCKETSERVERACCEPTERRORS_H
#define CHAUDIERE_TESTSOCKETSERVERACCEPTERRORS_H

#include "TestSuite.h"

namespace chaudiere
{

// Base fixture for TestSocketServerAcceptErrors's test cases (see
// TestSocketServerAcceptErrors.cpp), which are POIVRE_TEST_CASE self-registering test
// cases (see TestSocketSendInterrupt.h). Each test case gets its own freshly
// constructed instance, so this class carries no state of its own.
class TestSocketServerAcceptErrors : public poivre::TestSuite
{
public:
   TestSocketServerAcceptErrors();
};

}

#endif
