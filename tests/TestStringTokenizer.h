// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSTRINGTOKENIZER_H
#define CHAUDIERE_TESTSTRINGTOKENIZER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestStringTokenizer : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testConstructorWithDelimiter();
   void testHasMoreTokens();
   void testNextToken();
   void testCountTokens();
   
public:
   TestStringTokenizer();
   

};

}

#endif
