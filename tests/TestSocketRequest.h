// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTSOCKETREQUEST_H
#define TESTSOCKETREQUEST_H

#include "TestSuite.h"


class TestSocketRequest : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testRun();
   void testGetSocketFD();
   void testGetSocket();
   void testRequestComplete();

public:
   TestSocketRequest();
   

};

#endif
