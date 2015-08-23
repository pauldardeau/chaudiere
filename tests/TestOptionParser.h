// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_OPTIONPARSER_H
#define CHAUDIERE_OPTIONPARSER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestOptionParser : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testAddBooleanOption();
   void testAddOption();
   void testHasOption();
   void testGetOptionValue();
   void testHasBooleanOption();
   void testParseArgs();
   
public:
   TestOptionParser();

};

}

#endif
