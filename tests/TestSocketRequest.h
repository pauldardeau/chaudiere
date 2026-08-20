// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSOCKETREQUEST_H
#define CHAUDIERE_TESTSOCKETREQUEST_H

#include "TestSuite.h"

namespace chaudiere
{

class TestSocketRequest : public poivre::TestSuite
{
protected:
   void runTests();

   void testConstructor();
   void testConstructorWithCompletionObserver();
   void testRun();
   void testGetSocketFD();
   void testGetSocket();
   void testRequestComplete();
   void testIsSocketOwned();
   void testSetSocketOwned();
   void testNotifyOnCompletion();

public:
   TestSocketRequest();

};

}

#endif
