// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSOCKET_H
#define CHAUDIERE_TESTSOCKET_H

#include "TestSuite.h"

namespace chaudiere
{

class TestSocket : public poivre::TestSuite
{
protected:
   void runTests();
   
   void testConstructorWithAddress();
   void testConstructorWithFD();
   void testConstructorWithCompletionObserver();
   
   void testSend();
   void testWriteWithBuffer();
   void testWriteWithString();
   
   void testReceive();
   void testRead();
   
   void testClose();
   void testIsOpen();
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
   
   void testReadLine();
   
   void testGetPeerIPAddress();
   
   void testGetPort();
   
   void testSetIncludeMessageSize();

public:
   TestSocket();   

};

}

#endif
