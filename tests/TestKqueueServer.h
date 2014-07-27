// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTKQUEUESERVER_H
#define TESTKQUEUESERVER_H

#include "TestSuite.h"


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

#endif
