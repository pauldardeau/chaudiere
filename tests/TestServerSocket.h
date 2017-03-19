// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSERVERSOCKET_H
#define CHAUDIERE_TESTSERVERSOCKET_H

#include "TestSuite.h"

namespace chaudiere
{

class TestServerSocket : public poivre::TestSuite
{
protected:
   void runTests();
   
   void testConstructor();
   void testAccept();
   void testClose();

public:
   TestServerSocket();
   

};

}

#endif
