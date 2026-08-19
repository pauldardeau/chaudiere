// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTREQUESTHANDLER_H
#define CHAUDIERE_TESTREQUESTHANDLER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestRequestHandler : public poivre::TestSuite
{
protected:
   void runTests();

   void testConstructorWithSocket();
   void testConstructorWithSocketRequest();
   void testSetThreadPooling();
   void testIsThreadPooling();
   void testGetSocket();
   void testIsSocketOwned();
   void testSetSocketOwned();
   void testNotifyOnCompletion();

public:
   TestRequestHandler();

};

}

#endif
