// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestSocket.h"
#include "Socket.h"

//******************************************************************************

TestSocket::TestSocket() :
   TestSuite("TestSocket")
{
}

//******************************************************************************

void TestSocket::runTests()
{
   testConstructorWithAddress();
   testConstructorWithFD();
   testConstructorWithCompletionObserver();
   testSend();
   testWriteWithBuffer();
   testWriteWithString();
   testReceive();
   testRead();
   testClose();
   testIsOpen();
   testIsConnected();
   testCloseConnection();
   testGetFileDescriptor();
   testRequestComplete();
   testSetUserIndex();
   testGetUserIndex();
   testSetTcpNoDelay();
   testGetTcpNoDelay();
   testSetSendBufferSize();
   testGetSendBufferSize();
   testSetReceiveBufferSize();
   testGetReceiveBufferSize();
   testSetKeepAlive();
   testGetKeepAlive();
   testReadLine();
   testGetPeerIPAddress();
   testGetPort();
   testSetIncludeMessageSize();
}

//******************************************************************************

void TestSocket::testConstructorWithAddress()
{
   TEST_CASE("testConstructorWithAddress");
}

//******************************************************************************

void TestSocket::testConstructorWithFD()
{
   TEST_CASE("testConstructorWithFD");
}

//******************************************************************************

void TestSocket::testConstructorWithCompletionObserver()
{
   TEST_CASE("testConstructorWithCompletionObserver");
}

//******************************************************************************

void TestSocket::testSend()
{
   TEST_CASE("testSend");
}

//******************************************************************************

void TestSocket::testWriteWithBuffer()
{
   TEST_CASE("testWriteWithBuffer");
}

//******************************************************************************

void TestSocket::testWriteWithString()
{
   TEST_CASE("testWriteWithString");
}

//******************************************************************************

void TestSocket::testReceive()
{
   TEST_CASE("testReceive");
}

//******************************************************************************

void TestSocket::testRead()
{
   TEST_CASE("testRead");
}

//******************************************************************************

void TestSocket::testClose()
{
   TEST_CASE("testClose");
}

//******************************************************************************

void TestSocket::testIsOpen()
{
   TEST_CASE("testIsOpen");
}

//******************************************************************************

void TestSocket::testIsConnected()
{
   TEST_CASE("testIsConnected");
}

//******************************************************************************

void TestSocket::testCloseConnection()
{
   TEST_CASE("testCloseConnection");
}

//******************************************************************************

void TestSocket::testGetFileDescriptor()
{
   TEST_CASE("testGetFileDescriptor");
}

//******************************************************************************

void TestSocket::testRequestComplete()
{
   TEST_CASE("testRequestComplete");
}

//******************************************************************************

void TestSocket::testSetUserIndex()
{
   TEST_CASE("testSetUserIndex");
}

//******************************************************************************

void TestSocket::testGetUserIndex()
{
   TEST_CASE("testGetUserIndex");
}

//******************************************************************************

void TestSocket::testSetTcpNoDelay()
{
   TEST_CASE("testSetTcpNoDelay");
}

//******************************************************************************

void TestSocket::testGetTcpNoDelay()
{
   TEST_CASE("testGetTcpNoDelay");
}

//******************************************************************************

void TestSocket::testSetSendBufferSize()
{
   TEST_CASE("testSetSendBufferSize");
}

//******************************************************************************

void TestSocket::testGetSendBufferSize()
{
   TEST_CASE("testGetSendBufferSize");
}

//******************************************************************************

void TestSocket::testSetReceiveBufferSize()
{
   TEST_CASE("testSetReceiveBufferSize");
}

//******************************************************************************

void TestSocket::testGetReceiveBufferSize()
{
   TEST_CASE("testGetReceiveBufferSize");
}

//******************************************************************************

void TestSocket::testSetKeepAlive()
{
   TEST_CASE("testSetKeepAlive");
}

//******************************************************************************

void TestSocket::testGetKeepAlive()
{
   TEST_CASE("testGetKeepAlive");
}

//******************************************************************************

void TestSocket::testReadLine()
{
   TEST_CASE("testReadLine");
}

//******************************************************************************

void TestSocket::testGetPeerIPAddress()
{
   TEST_CASE("testGetPeerIPAddress");
}

//******************************************************************************

void TestSocket::testGetPort()
{
   TEST_CASE("testGetPort");
}

//******************************************************************************

void TestSocket::testSetIncludeMessageSize()
{
   TEST_CASE("testSetIncludeMessageSize");
}

//******************************************************************************

