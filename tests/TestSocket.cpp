// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestSocket.h"
#include "Socket.h"

using namespace chaudiere;

//******************************************************************************

TestSocket::TestSocket() :
   TestSuite("TestSocket") {
}

//******************************************************************************

void TestSocket::runTests() {
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

void TestSocket::testConstructorWithAddress() {
   TEST_CASE("testConstructorWithAddress");
   //TODO: implement testConstructorWithAddress
}

//******************************************************************************

void TestSocket::testConstructorWithFD() {
   TEST_CASE("testConstructorWithFD");
   //TODO: implement testConstructorWithFD
}

//******************************************************************************

void TestSocket::testConstructorWithCompletionObserver() {
   TEST_CASE("testConstructorWithCompletionObserver");
   //TODO: implement testConstructorWithCompletionObserver
}

//******************************************************************************

void TestSocket::testSend() {
   TEST_CASE("testSend");
   //TODO: implement testSend
}

//******************************************************************************

void TestSocket::testWriteWithBuffer() {
   TEST_CASE("testWriteWithBuffer");
   //TODO: implement testWriteWithBuffer
}

//******************************************************************************

void TestSocket::testWriteWithString() {
   TEST_CASE("testWriteWithString");
   //TODO: implement testWriteWithString
}

//******************************************************************************

void TestSocket::testReceive() {
   TEST_CASE("testReceive");
   //TODO: implement testReceive
}

//******************************************************************************

void TestSocket::testRead() {
   TEST_CASE("testRead");
   //TODO: implement testRead
}

//******************************************************************************

void TestSocket::testClose() {
   TEST_CASE("testClose");
   //TODO: implement testClose
}

//******************************************************************************

void TestSocket::testIsOpen() {
   TEST_CASE("testIsOpen");
   //TODO: implement testIsOpen
}

//******************************************************************************

void TestSocket::testIsConnected() {
   TEST_CASE("testIsConnected");
   //TODO: implement testIsConnected
}

//******************************************************************************

void TestSocket::testCloseConnection() {
   TEST_CASE("testCloseConnection");
   //TODO: implement testCloseConnection
}

//******************************************************************************

void TestSocket::testGetFileDescriptor() {
   TEST_CASE("testGetFileDescriptor");
   //TODO: implement testGetFileDescriptor
}

//******************************************************************************

void TestSocket::testRequestComplete() {
   TEST_CASE("testRequestComplete");
   //TODO: implement testRequestComplete
}

//******************************************************************************

void TestSocket::testSetUserIndex() {
   TEST_CASE("testSetUserIndex");
   //TODO: implement testSetUserIndex
}

//******************************************************************************

void TestSocket::testGetUserIndex() {
   TEST_CASE("testGetUserIndex");
   //TODO: implement testGetUserIndex
}

//******************************************************************************

void TestSocket::testSetTcpNoDelay() {
   TEST_CASE("testSetTcpNoDelay");
   //TODO: implement testSetTcpNoDelay
}

//******************************************************************************

void TestSocket::testGetTcpNoDelay() {
   TEST_CASE("testGetTcpNoDelay");
   //TODO: implement testGetTcpNoDelay
}

//******************************************************************************

void TestSocket::testSetSendBufferSize() {
   TEST_CASE("testSetSendBufferSize");
   //TODO: implement testSetSendBufferSize
}

//******************************************************************************

void TestSocket::testGetSendBufferSize() {
   TEST_CASE("testGetSendBufferSize");
   //TODO: implement testGetSendBufferSize
}

//******************************************************************************

void TestSocket::testSetReceiveBufferSize() {
   TEST_CASE("testSetReceiveBufferSize");
   //TODO: implement testSetReceiveBufferSize
}

//******************************************************************************

void TestSocket::testGetReceiveBufferSize() {
   TEST_CASE("testGetReceiveBufferSize");
   //TODO: implement testGetReceiveBufferSize
}

//******************************************************************************

void TestSocket::testSetKeepAlive() {
   TEST_CASE("testSetKeepAlive");
   //TODO: implement testSetKeepAlive
}

//******************************************************************************

void TestSocket::testGetKeepAlive() {
   TEST_CASE("testGetKeepAlive");
   //TODO: implement testGetKeepAlive
}

//******************************************************************************

void TestSocket::testReadLine() {
   TEST_CASE("testReadLine");
   //TODO: implement testReadLine
}

//******************************************************************************

void TestSocket::testGetPeerIPAddress() {
   TEST_CASE("testGetPeerIPAddress");
   //TODO: implement testGetPeerIPAddress
}

//******************************************************************************

void TestSocket::testGetPort() {
   TEST_CASE("testGetPort");
   //TODO: implement testGetPort
}

//******************************************************************************

void TestSocket::testSetIncludeMessageSize() {
   TEST_CASE("testSetIncludeMessageSize");
   //TODO: implement testSetIncludeMessageSize
}

//******************************************************************************

