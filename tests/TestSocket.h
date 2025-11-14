// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSOCKET_H
#define CHAUDIERE_TESTSOCKET_H

#include "TestSuite.h"

namespace chaudiere
{

class TestSocket : public TestSuite
{
protected:
   void runTests();
   void setup();
   void tearDown();

   void testConstructorWithAddress();
   void testConstructorWithFD();
   void testConstructorWithCompletionObserver();

   void testCreateSocket();
   void testIsDescriptorBorrowed();
   void testSetLineInputBuffer();
   void testAppendLineInputBuffer();

   void testSend();
   void testWriteWithBuffer();
   void testWriteWithString();

   void testReceive();
   void testRead();
   void testReadSocket();
   void testReadLine();
   void testReadMsg();

   void testClose();
   void testIsConnected();
   void testCloseConnection();
   void testGetFileDescriptor();
   void testRequestComplete();

   void testSetUserIndex();
   void testGetUserIndex();

   void testSetTcpNoDelay();
   void testGetTcpNoDelay();

   void testSetSendBufferSize();
   void testGetSendBufferSize();

   void testSetReceiveBufferSize();
   void testGetReceiveBufferSize();

   void testSetKeepAlive();
   void testGetKeepAlive();

   void testGetPeerIPAddress();

   void testGetPort();

   void testSetIncludeMessageSize();

public:
   TestSocket();

};

}

#endif
