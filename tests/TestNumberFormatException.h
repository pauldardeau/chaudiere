// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTNUMBERFORMATEXCEPTION_H
#define CHAUDIERE_TESTNUMBERFORMATEXCEPTION_H

#include "TestSuite.h"

namespace chaudiere
{

class TestNumberFormatException : public poivre::TestSuite
{
protected:
   void runTests();

   void testDefaultConstructor();
   void testConstructorWithMessage();
   void testCopyConstructor();
   void testAssignmentOperator();
   void testGetType();

public:
   TestNumberFormatException();

};

}

#endif
