// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTKQUEUESERVER_H
#define CHAUDIERE_TESTKQUEUESERVER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestKqueueServer : public TestSuite
{
protected:
   void runTests();

   void testConstructor();
   void testInit();
   void testRun();
   void testNotifySocketComplete();

public:
   TestKqueueServer();


};

}

#endif
