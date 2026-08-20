// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTEPOLLSERVER_H
#define CHAUDIERE_TESTEPOLLSERVER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestEpollServer : public poivre::TestSuite
{
protected:
   void runTests();

   void testIsSupportedPlatform();
   void testConstructor();
   void testInitWithNullHandler();
   void testInitWithInvalidPort();
   void testInit();
   void testAddFileDescriptorForRead();
   void testRemoveFileDescriptorFromRead();
   void testGetKernelEventsAndEventAccessors();

public:
   TestEpollServer();

};

}

#endif
