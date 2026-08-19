// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSOCKETSERVER_H
#define CHAUDIERE_TESTSOCKETSERVER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestSocketServer : public poivre::TestSuite
{
protected:
   void runTests();

   void testConstructor();
   void testGetConfigDataSource();
   void testGetSocketSendBufferSize();
   void testGetSocketReceiveBufferSize();
   void testGetServerId();
   void testPlatformPointerSizeBits();
   void testGetSystemDateGMT();
   void testGetLocalDateTime();
   void testHasTrueValue();
   void testGetIntValue();
   void testReplaceVariables();
   void testServiceSocket();
   void testRunSocketServer();

public:
   TestSocketServer();

};

}

#endif
