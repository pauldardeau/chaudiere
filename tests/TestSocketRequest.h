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
   void testRun();
   void testGetSocketFD();
   void testGetSocket();
   void testRequestComplete();

public:
   TestSocketRequest();

};

}

#endif
