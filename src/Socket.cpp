// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>

#include "Socket.h"
#include "SocketCompletionObserver.h"
#include "BasicException.h"
#include "Logger.h"

static const std::string EMPTY_STRING = "";
static const std::string CRLF = "\r\n";
static const std::string LF = "\n";

static const int DEFAULT_BUFFER_SIZE = 1024;

using namespace chaudiere;

//******************************************************************************

int Socket::createSocket() {
   const int socketFD = ::socket(AF_INET, SOCK_STREAM, 0);
   if (socketFD == -1) {
      LOG_ERROR("unable to create socket (file descriptor)")
   }

   return socketFD;
}

//******************************************************************************

Socket::Socket(const std::string& address, int port) :
   m_completionObserver(nullptr),
   m_serverAddress(address),
   m_socketFD(-1),
   m_userIndex(-1),
   m_port(port),
   m_isConnected(false),
   m_includeMessageSize(false),
   m_borrowedDescriptor(false),
   m_inBufferSize(DEFAULT_BUFFER_SIZE),
   m_lastReadSize(0) {

   LOG_INSTANCE_CREATE("Socket")

   if (!open()) {
      throw BasicException("Unable to open socket");
   }
}

//******************************************************************************

Socket::Socket(int socketFD) :
   m_completionObserver(nullptr),
   m_socketFD(socketFD),
   m_userIndex(-1),
   m_port(-1),
   m_isConnected(true),  // a guess (we have no way of knowing for sure)
   m_includeMessageSize(false),
   m_borrowedDescriptor(true),
   m_inputBuffer(DEFAULT_BUFFER_SIZE),
   m_inBufferSize(DEFAULT_BUFFER_SIZE),
   m_lastReadSize(0) {

   LOG_INSTANCE_CREATE("Socket")
}

//******************************************************************************

Socket::Socket(SocketCompletionObserver* completionObserver, int socketFD) :
   m_completionObserver(completionObserver),
   m_socketFD(socketFD),
   m_userIndex(-1),
   m_port(-1),
   m_isConnected(true),  // a guess (we have no way of knowing for sure)
   m_includeMessageSize(false),
   m_borrowedDescriptor(true),
   m_inputBuffer(DEFAULT_BUFFER_SIZE),
   m_inBufferSize(DEFAULT_BUFFER_SIZE),
   m_lastReadSize(0) {

   LOG_INSTANCE_CREATE("Socket")
}

//******************************************************************************

Socket::~Socket() {
   LOG_INSTANCE_DESTROY("Socket")

   if (m_socketFD > -1) {
      ::close(m_socketFD);
   }
}

//******************************************************************************

void Socket::init() {
   setTcpNoDelay(true);
   m_isConnected = true;
}

//******************************************************************************

bool Socket::open() {
   m_socketFD = Socket::createSocket();

   if (m_socketFD < 0) {
      LOG_ERROR("unable to create a socket file descriptor")
      return false;
   }

   ::memset((void *) &m_serverAddr, 0, sizeof(m_serverAddr));

   m_serverAddr.sin_family = AF_INET;
   m_serverAddr.sin_port = htons(m_port);
   m_serverAddr.sin_addr.s_addr = inet_addr(m_serverAddress.c_str());

   const int rc = ::connect(m_socketFD,
                            (struct sockaddr *) &m_serverAddr,
                            sizeof(m_serverAddr));

   if (rc < 0) {
      return false;
   } else {
      init();
      return true;
   }
}

//******************************************************************************

ssize_t Socket::send(const char* sendBuffer, size_t bufferLength, int flags) {
   if ((m_socketFD < 0) || (! m_isConnected) || (nullptr == sendBuffer)) {
      return -1;
   }

   if (sendPayload((const char*)sendBuffer, bufferLength, flags)) {
      return bufferLength;
   } else {
      return -1;
   }
}

//******************************************************************************

ssize_t Socket::receive(char* receiveBuffer, ssize_t bufferLength, int flags) {
   if ((m_socketFD < 0) || (!m_isConnected) || (nullptr == receiveBuffer)) {
      return -1;
   }

   return recvPayload(receiveBuffer, bufferLength, flags);
}

//******************************************************************************

void Socket::closeConnection() {
   close();
}

//******************************************************************************

void Socket::close() {
   if (m_socketFD > -1) {
      shutdown(m_socketFD, SHUT_RDWR);
      ::close(m_socketFD);
      if (!m_borrowedDescriptor) {
         m_socketFD = -1;
      }
      m_isConnected = false;
   }
}

//******************************************************************************

bool Socket::isConnected() const {
   return m_isConnected;
}

//******************************************************************************

int Socket::getFileDescriptor() const {
   return m_socketFD;
}

//******************************************************************************

void Socket::requestComplete() {
   if (m_completionObserver) {
      m_completionObserver->notifySocketComplete(this);
   }
}

//******************************************************************************

void Socket::setUserIndex(int userIndex) {
   m_userIndex = userIndex;
}

//******************************************************************************

int Socket::getUserIndex() const {
   return m_userIndex;
}

//******************************************************************************

bool Socket::setTcpNoDelay(bool on) {
   int sockopt_arg = on ? 1 : 0;
   return (0 == ::setsockopt(m_socketFD,
                             IPPROTO_TCP,
                             TCP_NODELAY,
                             (char *) &sockopt_arg,
                             sizeof(sockopt_arg)));
}

//******************************************************************************

bool Socket::getTcpNoDelay() const {
   int result = 0;
   socklen_t len = sizeof(int);

   if (0 == ::getsockopt(m_socketFD,
                         IPPROTO_TCP,
                         TCP_NODELAY,
                         (char*) &result,
                         &len)) {
      return (result > 0 ? true : false);
   } else {
      // error
      return false;
   }
}

//******************************************************************************

bool Socket::setSendBufferSize(int size) {
   if ((size >= 0) && (m_socketFD > -1)) {
      int sockopt_arg = size;
      const int rc = ::setsockopt(m_socketFD,
                                  SOL_SOCKET,
                                  SO_SNDBUF,
                                  &sockopt_arg,
                                  sizeof(sockopt_arg));
      return rc == 0;
   } else {
      return false;
   }
}

//******************************************************************************

int Socket::getSendBufferSize() const {
   int result = 0;
   socklen_t len = sizeof(int);

   if (0 == ::getsockopt(m_socketFD,
                         SOL_SOCKET,
                         SO_SNDBUF,
                         (char*) &result,
                         &len)) {
      return result;
   } else {
      // error
      return -1;
   }
}

//******************************************************************************

bool Socket::setReceiveBufferSize(int size) {
   if ((size >= 0) && (m_socketFD > -1)) {
      int sockopt_arg = size;
      const int rc = ::setsockopt(m_socketFD,
                                  SOL_SOCKET,
                                  SO_RCVBUF,
                                  &sockopt_arg,
                                  sizeof(sockopt_arg));
      return rc == 0;
   } else {
      return false;
   }
}

//******************************************************************************

int Socket::getReceiveBufferSize() const {
   int result = 0;
   socklen_t len = sizeof(int);

   if (0 == ::getsockopt(m_socketFD,
                         SOL_SOCKET,
                         SO_RCVBUF,
                         (char*) &result,
                         &len)) {
      return result;
   } else {
      // error
      return -1;
   }
}

//******************************************************************************

bool Socket::setKeepAlive(bool on) {
   int sockopt_arg = on ? 1 : 0;
   return (0 == ::setsockopt(m_socketFD,
                             SOL_SOCKET,
                             SO_KEEPALIVE,
                             (char *) &sockopt_arg,
                             sizeof(sockopt_arg)));
}

//******************************************************************************

bool Socket::getKeepAlive() const {
   int result = 0;
   socklen_t len = sizeof(int);

   if (0 == ::getsockopt(m_socketFD,
                         SOL_SOCKET,
                         SO_KEEPALIVE,
                         (char*) &result,
                         &len)) {
      return (result > 0 ? true : false);
   } else {
      // error
      return false;
   }
}

//******************************************************************************

bool Socket::sendPayloadSize(uint16_t payloadSize) {
   if (m_includeMessageSize) {
      uint16_t nOrderSize = htons(payloadSize);

      //printf("sending payload size\n");

      ssize_t bytesSent = ::send(m_socketFD,
                                 &nOrderSize,
                                 sizeof(uint16_t),
                                 0);

      //printf("back from sending payload size\n");

      if (bytesSent == sizeof(uint16_t)) {
         return true;
      } else {
         return false;
      }
   } else {
      return true;
   }
}

//******************************************************************************

bool Socket::recvPayloadSize(uint16_t& payloadSize) {
   if (m_includeMessageSize) {
      uint16_t nOrderSize;

      //printf("reading payload size\n");

      ssize_t bytes_received = ::recv(m_socketFD,
                                      &nOrderSize,
                                      sizeof(uint16_t),
                                      0);
      if (bytes_received == sizeof(uint16_t)) {
         uint16_t hOrderSize = ntohs(nOrderSize);
         payloadSize = hOrderSize;
         return true;
      } else {
         return false;
      }
   } else {
      return true;
   }
}

//******************************************************************************

ssize_t Socket::recvPayload(char* buffer, ssize_t bufferSize, int flags) {
   ssize_t recvTotalBytes = bufferSize;

   if (m_includeMessageSize) {
      uint16_t payloadSize;
      if (recvPayloadSize(payloadSize)) {
         recvTotalBytes = payloadSize;
      } else {
         return false;
      }
   }

   char* pRecvBuffer = buffer;
   ssize_t totalBytesReceived = 0;
   ssize_t remainingBytes = recvTotalBytes;
   bool allIsGood = true;

   while ((totalBytesReceived < recvTotalBytes)  && allIsGood) {
      ssize_t bytesReceived = ::recv(m_socketFD,
                                     pRecvBuffer,
                                     remainingBytes,
                                     0);
      if (bytesReceived > 0) {
         totalBytesReceived += bytesReceived;
         pRecvBuffer += bytesReceived;
         remainingBytes -= bytesReceived;
      } else if (bytesReceived == 0) {
      } else {
         allIsGood = false;
      }
   }

   return totalBytesReceived;
}

//******************************************************************************

bool Socket::readLine(std::string& line) {
   int bufferSizeNeeded = 0;

   uint16_t payloadSize;

   if (recvPayloadSize(payloadSize)) {
      bufferSizeNeeded = payloadSize;
   } else {
      return false;
   }

   char buffer[65535];
   memset(buffer, 0, sizeof(buffer));

   char *pRecvBuffer = buffer;
   ssize_t totalBytesReceived = 0;
   ssize_t remainingBufferSize = sizeof(buffer);
   bool allIsGood = true;

   while ((totalBytesReceived < bufferSizeNeeded) && allIsGood) {
      ssize_t bytesReceived = ::recv(m_socketFD,
                                     pRecvBuffer,
                                     remainingBufferSize,
                                     0);
      if (bytesReceived > 0) {
         totalBytesReceived += bytesReceived;
         remainingBufferSize -= bytesReceived;
         pRecvBuffer += bytesReceived;
      } else if (bytesReceived == 0) {
      } else {
         allIsGood = false;
      }
   }

   if (!allIsGood) {
      return false;
   }

   for (ssize_t i = 0; i < totalBytesReceived; ++i) {
      if (buffer[i] == '\n') {
         buffer[i] = '\0';
         line = buffer;
         return true;
      }
   }

   return false;
   
   /*
   line.erase();

   int eolLength = 0;
   std::string::size_type posEOL = std::string::npos;

   // do we already have anything in our line input buffer?
   if (!m_lineInputBuffer.empty()) {
      // look for CRLF EOL
      const std::string::size_type posCRLF = m_lineInputBuffer.find(CRLF);

      // is there one?
      if (std::string::npos != posCRLF) {
         eolLength = 2;
         posEOL = posCRLF;
      } else {
         const std::string::size_type posLF = m_lineInputBuffer.find(LF);

         if (std::string::npos != posLF) {
            eolLength = 1;
            posEOL = posLF;
         }
      }

      // did we find the EOL?
      if ((eolLength > 0) && (std::string::npos != posEOL)) {
         // extract everything prior to EOL
         line = m_lineInputBuffer.substr(0, posEOL);

         // remove the current line from the line input buffer
         m_lineInputBuffer.erase(0, posEOL + eolLength);

         if (Logger::isLogging(Debug)) {
            LOG_DEBUG("Socket::readLine, using buffer line: (next line)")
            LOG_DEBUG(line)
            LOG_DEBUG("*** remaining input buffer on next line")
         }

         return true;
      } else {
         LOG_DEBUG("*** assigning full input buffer")

         // put our line input buffer contents into the current line
         line = m_lineInputBuffer;

         if (Logger::isLogging(Debug)) {
            LOG_DEBUG("Socket::readLine, using partial buffer: (next line)")
            LOG_DEBUG(line)
         }

         // empty our line input buffer
         m_lineInputBuffer.erase();
      }
   }

   ssize_t bytes = 0;
   int bytesToRead = 1023;
   char buffer[1024];
   char* posLF;
   char* posCRLF;
   char* pszEOL;
   eolLength = 0;

   do {
      posLF = nullptr;
      posCRLF = nullptr;
      pszEOL = nullptr;

      bytes = ::recv(m_socketFD,
                     buffer,
                     bytesToRead,
                     0);

      if (Logger::isLogging(Debug)) {
         char msg[128];
         ::snprintf(msg, 128, "readline: bytes from recv = %zd", bytes);
         LOG_DEBUG(msg)
         buffer[bytes] = '\0';
         LOG_DEBUG("just read from socket recv (next line)")
         LOG_DEBUG(buffer)
      }

      if (bytes <= 0) {  // error or connection closed by peer?
         if (bytes == 0) {
            // our connection has been closed by the other process. nothing
            // more we can do!!!
            LOG_DEBUG("connection reset by peer")
            close();
            return false;
         } else {
            if (EINTR == errno) {  // interrupted?
               // not really an error
               continue;
            } else {
               LOG_WARNING("recv returned an error")
            }
         }
         return false;
      }

      buffer[bytes] = '\0';
      posCRLF = ::strstr(buffer, "\r\n");

      if (posCRLF) {
         pszEOL = posCRLF;
         eolLength = 2;
      } else {
         posLF = ::strstr(buffer, "\n");

         if (posLF) {
            pszEOL = posLF;
            eolLength = 1;
         }
      }

      if ((nullptr != pszEOL) && (eolLength > 0)) {
         // hold onto everything after the EOL in the line input buffer
         m_lineInputBuffer = (pszEOL + eolLength);

         if (Logger::isLogging(Debug)) {
            LOG_DEBUG("Socket::readLine holding onto following text in line input buffer")
            char msg[256];
            ::snprintf(msg, 256, "buffer: '%s'", m_lineInputBuffer.c_str());
            LOG_DEBUG(msg)
         }

         // terminate the string at the EOL
         *pszEOL = '\0';
      }

      line += buffer;

   } while (nullptr == pszEOL);

   return true;
   */
}

//******************************************************************************

bool Socket::read(char* buffer, int bufsize) {
   if ((m_socketFD < 0) || (!m_isConnected) || (nullptr == buffer)) {
      return false;
   }

   ssize_t bytesReceived = recvPayload(buffer, bufsize, 0);
   return bytesReceived > 0;

   /*
   int length;
   int bytesAlreadyRead = 0;

   if (!m_lineInputBuffer.empty()) {
      const int nInputBufferLen = (int) m_lineInputBuffer.length();

      if (nInputBufferLen >= bufsize) {
         // we don't have to read from the socket, we've got the full amount
         // in our line input buffer
         ::memcpy(buffer, m_lineInputBuffer.c_str(), bufsize);
         m_lineInputBuffer.erase(0, bufsize);
         return true;
      } else {
         // empty the line input buffer
         ::memcpy(buffer, m_lineInputBuffer.c_str(), nInputBufferLen);
         m_lineInputBuffer = EMPTY_STRING;
         length = bufsize - nInputBufferLen;
         bytesAlreadyRead = nInputBufferLen;
      }
   } else {
      length = bufsize;
   }

   if (length <= 0) {
      LOG_WARNING("Socket::read failed, length <= 0")
      return (bytesAlreadyRead > 0);
   }

   if (bufsize < length) {
      LOG_WARNING("Socket::read failed, bufsize < length")
      return (bytesAlreadyRead > 0);
   }

   if (!readMsg(length)) {
      LOG_WARNING("Socket::read failed, readMsg returned false")
      return (bytesAlreadyRead > 0);
   }

   ::memcpy(buffer + bytesAlreadyRead, m_inputBuffer.data(), length);

   return true;
   */
}

//******************************************************************************

bool Socket::readMsg(int length) {
   if (!isConnected()) {
      LOG_WARNING("unable to read message size, socket is closed")
      return false;
   }

   m_inputBuffer.ensureCapacity(m_inBufferSize);
   int bytes_read = readSocket(m_inputBuffer.data(), length);
   if (bytes_read == length) {
      m_inputBuffer.data()[length] = '\0';
      m_lastReadSize = length;
      return true;
   } else {
      LOG_ERROR("readSocket failed")
      return false;
   }
}

//******************************************************************************

int Socket::readSocket(char* buffer, int bytesToRead) {
   if ((m_socketFD < 0) || (!m_isConnected) || (nullptr == buffer)) {
      return -1;
   }

   return recvPayload(buffer, bytesToRead, 0);

   /*
   int total_bytes_rcvd = 0;
   ssize_t bytes = 0;
   char* currentBufferDest = buffer;

   do {
      bytes = ::recv(m_socketFD,
                     currentBufferDest,
                     bytesToRead - total_bytes_rcvd,
                     0);

      if (Logger::isLogging(LogLevel::Debug)) {
         char msg[128];
         ::snprintf(msg, 128, "recv, bytes from recv = %ld", bytes);
         LOG_DEBUG(msg)
      }

      if (bytes <= 0) {  // error or connection closed by peer?
         if (bytes == 0) {
            // our connection has been closed by the other process. nothing
            // more we can do!!!
            LOG_DEBUG("connection reset by peer")
            close();
            if (total_bytes_rcvd == 0) {
               return -1;
            } else {
               return total_bytes_rcvd;
            }
         } else {
            if (EINTR == errno) {  // interrupted?
               // not really an error
               continue;
            } else {
               LOG_WARNING("recv returned an error")
            }
         }

         if (total_bytes_rcvd == 0) {
            return -1;
         } else {
            return total_bytes_rcvd;
         }
      }

      total_bytes_rcvd += bytes;
      currentBufferDest += bytes;

   } while (total_bytes_rcvd < bytesToRead);

   return total_bytes_rcvd;
   */
}

//******************************************************************************

bool Socket::getPeerIPAddress(std::string& ipAddress) {
   struct sockaddr_in addr;
   socklen_t x = sizeof(addr);

   if (!::getpeername(m_socketFD, (struct sockaddr*) &addr, &x)) {
      char ipAddressBuffer[64];
      memset(ipAddressBuffer, 0, sizeof(ipAddressBuffer));
      ::inet_ntop(AF_INET,
                  &addr.sin_addr,
                  ipAddressBuffer,
                  sizeof(ipAddressBuffer));
      ipAddress = ipAddressBuffer;
      return true;
   } else {
      return false;
   }
}

//******************************************************************************

bool Socket::sendPayload(const char* buffer, ssize_t payloadSize, int flags) {
   if (m_includeMessageSize) {
      if (payloadSize <= 65535) {
         uint16_t sendSize = (uint16_t) payloadSize;
         if (!sendPayloadSize(sendSize)) {
            return false;
         }
      } else {
         return false;
      }
   }

   ssize_t totalBytesSent = 0;
   const char* pSendBuffer = buffer;
   ssize_t remainingBytes = payloadSize;
   bool allIsGood = true;

   while ((totalBytesSent < payloadSize) && allIsGood) {
      int bytesSent = ::send(m_socketFD,
                             pSendBuffer,
                             remainingBytes,
                             flags);
      if (bytesSent > 0) {
         totalBytesSent += bytesSent;
         pSendBuffer += bytesSent;
         remainingBytes -= bytesSent;
      } else if (bytesSent == 0) {
      } else {
         allIsGood = false;
      }
   }

   return allIsGood;
}

//******************************************************************************

bool Socket::write(const char* buffer, unsigned long bufsize) {
   if (!isConnected()) {
      LOG_WARNING("unable to write message, socket is closed")
      return false;
   }

   return sendPayload(buffer, bufsize, 0);
}

//******************************************************************************

bool Socket::write(const std::string& payload) {
   return write(payload.c_str(), payload.length());
}

//******************************************************************************

int Socket::getPort() const {
   return m_port;
}

//******************************************************************************

void Socket::setIncludeMessageSize(bool isSizeIncluded) {
   m_includeMessageSize = isSizeIncluded;
}

//******************************************************************************

bool Socket::getIncludeMessageSize() const {
   return m_includeMessageSize;
}

//******************************************************************************

void Socket::setLineInputBuffer(const std::string& s) {
   m_lineInputBuffer = s;
}

//******************************************************************************

void Socket::appendLineInputBuffer(const std::string& s) {
   m_lineInputBuffer += s;
}

//******************************************************************************

bool Socket::isDescriptorBorrowed() const {
   return m_borrowedDescriptor;
}

//******************************************************************************

