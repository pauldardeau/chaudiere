// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTKEYVALUEPAIRS_H
#define CHAUDIERE_TESTKEYVALUEPAIRS_H

#include "TestSuite.h"

namespace chaudiere
{

class TestKeyValuePairs : public TestSuite
{
protected:
   void runTests();

   void testConstructor();
   void testCopyConstructor();
   void testAssignmentCopy();
   void testGetKeys();
   void testHasKey();
   void testGetValue();
   void testAddPair();
   void testRemovePair();
   void testClear();
   void testSize();
   void testEmpty();

public:
   TestKeyValuePairs();

};

}

#endif
