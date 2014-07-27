// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTSERVERSOCKET_H
#define TESTSERVERSOCKET_H

#include "TestSuite.h"


class TestServerSocket : public TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testAccept();
   void testClose();

public:
   TestServerSocket();
   

};

#endif
