// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTINVALIDKEYEXCEPTION_H
#define CHAUDIERE_TESTINVALIDKEYEXCEPTION_H

#include "TestSuite.h"

namespace chaudiere
{

class TestInvalidKeyException : public poivre::TestSuite
{
protected:
   void runTests();

   void testConstructor();
   void testCopyConstructor();

   void testAssignmentCopy();

   void testGetKey();

public:
   TestInvalidKeyException();
};

}

#endif
