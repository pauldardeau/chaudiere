// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTINVALIDKEYEXCEPTION_H
#define TESTINVALIDKEYEXCEPTION_H

#include "TestSuite.h"


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

#endif
