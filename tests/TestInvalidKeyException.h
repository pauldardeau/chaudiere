// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTINVALIDKEYEXCEPTION_H
#define CHAUDIERE_TESTINVALIDKEYEXCEPTION_H

#include "TestSuite.h"

namespace chaudiere
{

class TestInvalidKeyException : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testCopyConstructor();
   void testMoveConstructor();
   
   void testAssignmentCopy();
   void testAssignmentMove();
   
   void testGetKey();
   
public:
   TestInvalidKeyException();
};

}

#endif
