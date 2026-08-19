// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#ifndef CHAUDIERE_TESTSYSTEMINFO_H
#define CHAUDIERE_TESTSYSTEMINFO_H

#include "TestSuite.h"

namespace chaudiere
{

class TestSystemInfo : public poivre::TestSuite
{
protected:
   void runTests();

   void testConstructor();
   void testCopyConstructor();
   void testAssignmentOperator();
   void testSysName();
   void testNodeName();
   void testRelease();
   void testVersion();
   void testMachine();
   void testRetrievedSystemInfo();

public:
   TestSystemInfo();


};

}

#endif
