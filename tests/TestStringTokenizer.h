// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTSTRINGTOKENIZER_H
#define TESTSTRINGTOKENIZER_H

#include "TestSuite.h"


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

#endif
