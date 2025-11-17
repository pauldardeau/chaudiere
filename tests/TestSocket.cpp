// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <memory>
#include <errno.h>
#include <unistd.h>

#include "TestSocket.h"
#include "Socket.h"
#include "ServerSocket.h"
#include "BasicException.h"
#include "Runnable.h"

using namespace std;
using namespace chaudiere;

const string TEST_SOCKET_HOST = "127.0.0.1";
constexpr int TEST_SOCKET_PORT = 3339;


class EchoSocketServer
{
private:
   int m_serverSocket;
   int m_port;
   bool m_bindAndListenSuccess;
   bool m_isRunning;
   pthread_t m_threadHandle;

public:
   EchoSocketServer() :
      m_serverSocket(-1),
      m_port(0),
      m_bindAndListenSuccess(false),
      m_isRunning(false),
      m_threadHandle(0) {
   }

   ~EchoSocketServer() {
      if (m_serverSocket > -1) {
         shutdown(m_serverSocket, SHUT_RDWR);
         close(m_serverSocket);
      }
   }

   void stop() {
      m_isRunning = false;
   }

   bool init(int port) {
      m_port = port;
      m_serverSocket = Socket::createSocket();
      if (m_serverSocket > -1) {
         ServerSocket::setReuseAddr(m_serverSocket);

         //printf("binding on port %d\n", m_port);

         bool bindSuccess = ServerSocket::bind(m_serverSocket, m_port);
         bool listenSuccess = ServerSocket::listen(m_serverSocket, 10);

         m_bindAndListenSuccess = bindSuccess && listenSuccess;

         if (m_bindAndListenSuccess) {
            //printf("m_bindAndListenSuccess is true\n");
         } else {
            printf("m_bindAndListenSuccess is false\n");
         }

         return m_bindAndListenSuccess;
      } else {
         return false;
      }
   }

   bool runThread() {
      if (m_bindAndListenSuccess) {
         m_isRunning = true;

         //printf("calling pthread_create\n");

         if (0 == pthread_create(&m_threadHandle, 0, runThreadFn, this)) {
            return true;
         } else {
            m_isRunning = false;
         }
      } else {
         printf("runThread not creating thread b/c m_bindAndListenSuccess if false\n");
      }
      return false;
   }

   static string errorToString(int errValue) {
      string errString;

      switch (errValue) {
         case EAGAIN:  // EWOULDBLOCK
            errString = "EAGAIN/EWOULDBLOCK";
            break;
         case EBADF:
            errString = "EBADF";
            break;
         case ECONNREFUSED:
            errString = "ECONNREFUSED";
            break;
         case EFAULT:
            errString = "EFAULT";
            break;
         case EINTR:
            errString = "EINTR";
            break;
         case EINVAL:
            errString = "EINVAL";
            break;
         case ENOMEM:
            errString = "ENOMEM";
            break;
         case ENOTCONN:
            errString = "ENOTCONN";
            break;
         case ENOTSOCK:
            errString = "ENOTSOCK";
            break;
      }

      return errString;
   }

   static void* runThreadFn(void* pArgs) {

      //printf("### runThreadFn entered\n");

      EchoSocketServer* socketServer = (EchoSocketServer*) pArgs;

      struct sockaddr_in clientAddr;
      socklen_t namelen = sizeof(clientAddr);
      bool allIsGood = true;

      while (socketServer->m_isRunning && allIsGood) {

         //printf("server thread waiting on connection\n");

         int connectionSocket = accept(socketServer->m_serverSocket,
                                       (struct sockaddr*) &clientAddr,
                                       &namelen);
         if (connectionSocket < 0) {
            //int err = errno;
            //printf("accept error: %s\n", errorToString(err).c_str());

            allIsGood = false;
            continue;
         }

         //printf("client connection made\n");

         if (!socketServer->m_isRunning) {
            if (connectionSocket > -1) {
               shutdown(connectionSocket, SHUT_RDWR);
               close(connectionSocket);
            }
            continue;
         }

         // read payload size (2 bytes - uint16)
         uint16_t nOrderSize;
         ssize_t bytesReceived = recv(connectionSocket, &nOrderSize, sizeof(uint16_t), 0);
         if (bytesReceived == sizeof(uint16_t)) {
            uint16_t hOrderSize = htons(nOrderSize);

            // if payload size > 0
            if (hOrderSize > 0) {
               char recvBuffer[65535];
               memset(recvBuffer, 0, sizeof(recvBuffer));
               ssize_t totalBytesReceived = 0;
               char* pBufferReceive = recvBuffer;
               ssize_t remainingBufferBytes = sizeof(recvBuffer);
               bool allIsGood = true;

               while ((totalBytesReceived < hOrderSize) && allIsGood) {
                  ssize_t bytesReceived =
                     recv(connectionSocket, pBufferReceive, remainingBufferBytes, 0);
                  if (bytesReceived > 0) {
                     totalBytesReceived += bytesReceived;
                     remainingBufferBytes -= bytesReceived;
                     pBufferReceive += bytesReceived;
                  } else if (bytesReceived == 0) {
                  } else {
                     // error
                     const int err = errno;
                     printf("error: %s\n", errorToString(err).c_str());
                     close(connectionSocket);
                     allIsGood = false;
                  }
               }

               if ((totalBytesReceived == hOrderSize) && allIsGood) {
                  // write payload size
                  ssize_t bytesWritten = send(connectionSocket, &nOrderSize, sizeof(uint16_t), 0);

                  if (bytesWritten == sizeof(uint16_t)) {
                     // write payload
                     char* pWriteBuffer = recvBuffer;
                     ssize_t remainingBytes = hOrderSize;
                     ssize_t totalBytesWritten = 0;
                     while ((totalBytesWritten < hOrderSize) && allIsGood) {
                        ssize_t bytesWritten =
                           send(connectionSocket, pWriteBuffer, remainingBytes, 0);
                        if (bytesWritten > 0) {
                           totalBytesWritten += bytesWritten;
                           pWriteBuffer += bytesWritten;
                           remainingBytes -= bytesWritten;
                        } else if (bytesWritten == 0) {
                        } else {
                           // error
                           const int err = errno;
                           printf("error: %s\n", errorToString(err).c_str());
                           close(connectionSocket);
                           allIsGood = false;
                        }
                     }
                  }
               }
            }
         }
      }

      //printf("### runThreadFn exiting\n");

      return nullptr;
   }
};

static EchoSocketServer* echoSocketServer = nullptr;

//******************************************************************************

Socket* create_local_client_socket() {
   Socket* s = new Socket(TEST_SOCKET_HOST, TEST_SOCKET_PORT);
   s->setIncludeMessageSize(true);
   return s;
}

//******************************************************************************

TestSocket::TestSocket() :
   TestSuite("TestSocket") {
}

//******************************************************************************

void TestSocket::setup() {
   //printf("TestSocket::setup\n");

   echoSocketServer = new EchoSocketServer;
   if (echoSocketServer->init(TEST_SOCKET_PORT)) {
      if (echoSocketServer->runThread()) {
         //printf("EchoSocketServer::runThread success\n");
      } else {
         printf("error: EchoSocketServer::runThread failed\n");
         delete echoSocketServer;
         echoSocketServer = nullptr;
      }
   } else {
      printf("error: EchoSocketServer::init failed\n");
      delete echoSocketServer;
      echoSocketServer = nullptr;
   }
}

//******************************************************************************

void TestSocket::tearDown() {
   //printf("TestSocket::tearDown\n");

   if (echoSocketServer != nullptr) {
      echoSocketServer->stop();
      unique_ptr<Socket> s(create_local_client_socket());
      // don't send any data, just close
      int socketFD = s->getFileDescriptor();
      shutdown(socketFD, SHUT_RDWR);
      s->close();
      delete echoSocketServer;
      echoSocketServer = nullptr;
      sleep(1);
   }
}

//******************************************************************************

void TestSocket::runTests() {
   testConstructorWithAddress();
   testConstructorWithFD();
   testConstructorWithCompletionObserver();
   testCreateSocket();
   testIsDescriptorBorrowed();
   testSetLineInputBuffer();
   testAppendLineInputBuffer();
   testSend();
   testWriteWithBuffer();
   testWriteWithString();
   testReceive();
   testRead();
   testReadSocket();
   testReadLine();
   testReadMsg();
   testClose();
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
   testSetUserIndex();
   testGetUserIndex();
   testReadLine();
   testGetPeerIPAddress();
   testGetPort();
   testSetIncludeMessageSize();
}

//******************************************************************************

void TestSocket::testConstructorWithAddress() {
   TEST_CASE("testConstructorWithAddress");

   // create a connected socket
   try {
      unique_ptr<Socket> s(create_local_client_socket());
      require(s->isConnected(), "isConnected should return true");
      require(TEST_SOCKET_PORT == s->getPort(), "port should match");
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }

   class BadSocketCreator : public Runnable {
   private:
      string m_ipOrHost;
      int m_port;

   public:
      BadSocketCreator(const string& ipOrHost, int port) :
         m_ipOrHost(ipOrHost),
         m_port(port) {
      }

      void run() {
         Socket s(m_ipOrHost, m_port);
      }
   };

   // use a bad ip address
   Runnable* runnable = new BadSocketCreator("12q.0.0.1", TEST_SOCKET_PORT);
   requireException("BasicException", runnable);

   // use a bad hostname
   runnable = new BadSocketCreator("asdfklasdfl;kjasdf", TEST_SOCKET_PORT);
   requireException("BasicException", runnable);

   // use an invalid port
   runnable = new BadSocketCreator(TEST_SOCKET_HOST, TEST_SOCKET_PORT+1);
   requireException("BasicException", runnable);
}

//******************************************************************************

void TestSocket::testConstructorWithFD() {
   TEST_CASE("testConstructorWithFD");

   int socket_fd = Socket::createSocket();
   {
      Socket s(socket_fd);
   }
   ::close(socket_fd);
}

//******************************************************************************

void TestSocket::testConstructorWithCompletionObserver() {
   TEST_CASE("testConstructorWithCompletionObserver");
   //TODO: implement testConstructorWithCompletionObserver
}

//******************************************************************************

void TestSocket::testCreateSocket() {
   TEST_CASE("testCreateSocket");

   const int fileDescriptor = Socket::createSocket();
   require(fileDescriptor > -1);
   if (fileDescriptor > -1) {
      ::close(fileDescriptor);
   }
}

//******************************************************************************

void TestSocket::testIsDescriptorBorrowed() {
   TEST_CASE("testIsDescriptorBorrowed");
   //TODO: implement testIsDescriptorBorrowed
}

//******************************************************************************

void TestSocket::testSetLineInputBuffer() {
   TEST_CASE("testSetLineInputBuffer");
   //TODO: implement testSetLineInputBuffer
}

//******************************************************************************

void TestSocket::testAppendLineInputBuffer() {
   TEST_CASE("testAppendLineInputBuffer");
   //TODO: implement testAppendLineInputBuffer
}

//******************************************************************************

void TestSocket::testSend() {
   TEST_CASE("testSend");

   try {
      unique_ptr<Socket> s(create_local_client_socket());
      char reqPayload[] = "Hello from client socket!\n";
      char expRespPayload[] = "Hello from client socket!";
      ssize_t bytesSent = s->send(reqPayload, sizeof(reqPayload), 0);
      string respPayload;
      require(s->readLine(respPayload));
      requireStringEquals(expRespPayload, respPayload);
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testWriteWithBuffer() {
   TEST_CASE("testWriteWithBuffer");

   try {
      unique_ptr<Socket> s(create_local_client_socket());
      char reqPayload[] = "Hello from client socket!\n";
      char expRespPayload[] = "Hello from client socket!";
      s->write(reqPayload, sizeof(reqPayload));
      string respPayload;
      require(s->readLine(respPayload));
      requireStringEquals(expRespPayload, respPayload);
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testWriteWithString() {
   TEST_CASE("testWriteWithString");

   try {
      unique_ptr<Socket> s(create_local_client_socket());
      string reqPayload = "Hello from client socket!\n";
      string expRespPayload = "Hello from client socket!";
      s->write(reqPayload);
      string respPayload;
      require(s->readLine(respPayload));
      requireStringEquals(expRespPayload, respPayload);
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testReceive() {
   TEST_CASE("testReceive");

   try {
      unique_ptr<Socket> s(create_local_client_socket());
      string reqPayload = "Hello from client socket!";
      s->write(reqPayload);

      char receiveBuffer[32];
      memset(receiveBuffer, 0, sizeof(receiveBuffer));

      ssize_t bytesReceived = s->receive(receiveBuffer, sizeof(receiveBuffer), 0);

      string actPayload = receiveBuffer;

      require(bytesReceived == reqPayload.length());
      requireStringEquals(reqPayload, actPayload);
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testRead() {
   TEST_CASE("testRead");

   try {
      unique_ptr<Socket> s(create_local_client_socket());
      char reqPayload[] = "Hello from client socket!";
      ssize_t bytesSent = s->send(reqPayload, sizeof(reqPayload), 0);
      char buffer[34];
      memset(buffer, 0, sizeof(buffer));
      bool readSuccess = s->read(buffer, sizeof(buffer));
      string actPayload = buffer;
      require(readSuccess, "read should succeed");
      requireStringEquals(string(reqPayload), actPayload, "strings should be equal");
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testReadSocket() {
   TEST_CASE("testReadSocket");

   try {
      unique_ptr<Socket> s(create_local_client_socket());
      char reqPayload[] = "Hello from client socket!";
      ssize_t bytesSent = s->send(reqPayload, sizeof(reqPayload)-1, 0);
      char buffer[34];
      memset(buffer, 0, sizeof(buffer));
      const int bytesRead = s->readSocket(buffer, sizeof(buffer));
      string actPayload = buffer;
      require(bytesRead == strlen(reqPayload), "read should return exp number of bytes");
      requireStringEquals(string(reqPayload), actPayload, "strings should be equal");
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testReadLine() {
   TEST_CASE("testReadLine");

   try {
      unique_ptr<Socket> s(create_local_client_socket());
      char reqPayload[] = "Hello from client socket!\n";
      char expRespPayload[] = "Hello from client socket!";
      ssize_t bytesSent = s->send(reqPayload, sizeof(reqPayload), 0);
      string respPayload;
      require(s->readLine(respPayload));
      requireStringEquals(expRespPayload, respPayload);
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testReadMsg() {
   TEST_CASE("testReadMsg");
   //TODO: implement testReadMsg
}

//******************************************************************************

void TestSocket::testClose() {
   TEST_CASE("testClose");

   // create a disconnected socket
   Socket s1(Socket::createSocket());
   s1.close();

   // create a connected socket
   try {
      Socket s2(TEST_SOCKET_HOST, TEST_SOCKET_PORT);
      require(s2.isConnected());
      s2.close();
      requireFalse(s2.isConnected());
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testIsConnected() {
   TEST_CASE("testIsConnected");

   // create a disconnected socket
   Socket s1(Socket::createSocket());
   s1.close();
   requireFalse(s1.isConnected());

   // create a connected socket
   try {
      Socket s2(TEST_SOCKET_HOST, TEST_SOCKET_PORT);
      require(s2.isConnected());
      s2.close();
      requireFalse(s2.isConnected());
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testCloseConnection() {
   TEST_CASE("testCloseConnection");

   // create a disconnected socket
   Socket s1(Socket::createSocket());
   s1.closeConnection();

   // create a connected socket
   try {
      Socket s2(TEST_SOCKET_HOST, TEST_SOCKET_PORT);
      require(s2.isConnected());
      s2.closeConnection();
      requireFalse(s2.isConnected());
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testGetFileDescriptor() {
   TEST_CASE("testGetFileDescriptor");

   const int fileDescriptor = Socket::createSocket();
   Socket s(fileDescriptor);
   require(fileDescriptor == s.getFileDescriptor());
}

//******************************************************************************

void TestSocket::testRequestComplete() {
   TEST_CASE("testRequestComplete");

   //TODO: implement testRequestComplete
}

//******************************************************************************

void TestSocket::testSetUserIndex() {
   TEST_CASE("testSetUserIndex");

   Socket s(Socket::createSocket());
   const int userIndex = 23;
   s.setUserIndex(userIndex);
   require(userIndex == s.getUserIndex());
}

//******************************************************************************

void TestSocket::testGetUserIndex() {
   TEST_CASE("testGetUserIndex");

   Socket s(Socket::createSocket());
   require(-1 == s.getUserIndex());

   const int userIndex = 17;
   s.setUserIndex(userIndex);
   require(userIndex == s.getUserIndex());
}

//******************************************************************************

void TestSocket::testSetTcpNoDelay() {
   TEST_CASE("testSetTcpNoDelay");

   Socket s(Socket::createSocket());
   bool tcpNoDelay = true;
   require(s.setTcpNoDelay(tcpNoDelay), "setTcpNoDelay should not error");
   require(tcpNoDelay == s.getTcpNoDelay());

   tcpNoDelay = false;
   require(s.setTcpNoDelay(tcpNoDelay), "setTcpNoDelay should not error");
   require(tcpNoDelay == s.getTcpNoDelay());
}

//******************************************************************************

void TestSocket::testGetTcpNoDelay() {
   TEST_CASE("testGetTcpNoDelay");

   // testSetTcpNoDelay tests getTcpNoDelay
}

//******************************************************************************

void TestSocket::testSetSendBufferSize() {
   TEST_CASE("testSetSendBufferSize");

   try {
      unique_ptr<Socket> s(create_local_client_socket());

      for (int bufferSize = 1024; bufferSize < 32767; bufferSize += 1024) {
         require(s->setSendBufferSize(bufferSize), "setSendBufferSize should succeed");
         const int actBufferSize = s->getSendBufferSize();
         require(actBufferSize >= bufferSize, "getSendBufferSize() should match");
      }

      requireFalse(s->setSendBufferSize(-1024), "negative buffer size should error");
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testGetSendBufferSize() {
   TEST_CASE("testGetSendBufferSize");

   // testSetSendBufferSize also tests getSendBufferSize
}

//******************************************************************************

void TestSocket::testSetReceiveBufferSize() {
   TEST_CASE("testSetReceiveBufferSize");

   try {
      unique_ptr<Socket> s(create_local_client_socket());

      for (int bufferSize = 1024; bufferSize < 32767; bufferSize += 1024) {
         require(s->setReceiveBufferSize(bufferSize), "setReceiveBufferSize should succeed");
         const int actBufferSize = s->getReceiveBufferSize();
         require(actBufferSize >= bufferSize, "getReceiveBufferSize should match");
      }

      requireFalse(s->setReceiveBufferSize(-1024), "negative buffer size should error");
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testGetReceiveBufferSize() {
   TEST_CASE("testGetReceiveBufferSize");

   // testSetReceiveBufferSize also tests getReceiveBufferSize
}

//******************************************************************************

void TestSocket::testSetKeepAlive() {
   TEST_CASE("testSetKeepAlive");

   Socket s(Socket::createSocket());
   bool keepAlive = true;
   require(s.setKeepAlive(keepAlive), "setKeepAlive should not error");
   require(keepAlive == s.getKeepAlive());

   keepAlive = false;
   require(s.setKeepAlive(keepAlive), "setKeepAlive should not error");
   require(keepAlive == s.getKeepAlive());
}

//******************************************************************************

void TestSocket::testGetKeepAlive() {
   TEST_CASE("testGetKeepAlive");

   // testSetKeepAlive also tests testGetKeepAlive
}

//******************************************************************************

void TestSocket::testGetPeerIPAddress() {
   TEST_CASE("testGetPeerIPAddress");

   // create a disconnected socket
   Socket s1(Socket::createSocket());
   string peerIPAddress;
   requireFalse(s1.getPeerIPAddress(peerIPAddress), "disconnected socket shouldn't have a peer address");
   require(peerIPAddress.empty(), "peerIPAddress should still be empty");

   // create a connected socket
   try {
      string expPeerIPAddress = TEST_SOCKET_HOST;
      Socket s2(expPeerIPAddress, TEST_SOCKET_PORT);
      string actPeerIPAddress;
      require(s2.getPeerIPAddress(actPeerIPAddress));
      requireStringEquals(expPeerIPAddress, actPeerIPAddress);
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testGetPort() {
   TEST_CASE("testGetPort");

   // create a disconnected socket
   Socket s1(Socket::createSocket());
   require(-1 == s1.getPort());

   // create a connected socket
   try {
      int expPort = TEST_SOCKET_PORT;
      Socket s2(TEST_SOCKET_HOST, expPort);
      int actPort = s2.getPort();
      require(expPort == actPort);
   } catch (const BasicException& be) {
      failTest(string("BasicException caught: ") + be.whatString());
   } catch (const std::exception& e) {
      failTest(string("std::exception caught: ") + e.what());
   } catch (...) {
      failTest("unknown exception caught");
   }
}

//******************************************************************************

void TestSocket::testSetIncludeMessageSize() {
   TEST_CASE("testSetIncludeMessageSize");

   //TODO: implement testSetIncludeMessageSize
}

//******************************************************************************

