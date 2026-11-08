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
#include <sys/time.h>
#include <atomic>
#include <chrono>
#include <thread>

#include "Socket.h"
#include "SocketCompletionObserver.h"
#include "BasicException.h"
#include "Logger.h"
#include "internal/SocketIo.h"

static const std::string EMPTY_STRING = "";
static const std::string CRLF = "\r\n";
static const std::string LF = "\n";

static const int DEFAULT_BUFFER_SIZE = 1024;

// Writing to a connection the peer has closed must make the write fail with EPIPE, not
// terminate the process with SIGPIPE, and the process-wide SIGPIPE handling must stay
// as the application set it. Two socket-local mechanisms exist (see the class comment
// in Socket.h): MSG_NOSIGNAL, a flag of each send, and SO_NOSIGPIPE, an option of the
// socket. Every send below carries the flag where it exists, and the option is set on
// every descriptor a Socket manages where that exists.
#if !defined(MSG_NOSIGNAL) && !defined(SO_NOSIGPIPE) && !defined(CHAUDIERE_NO_SIGPIPE_PROTECTION)
#error "Socket.cpp needs MSG_NOSIGNAL or SO_NOSIGPIPE to keep a write to a closed connection from raising SIGPIPE; this platform has neither. Define CHAUDIERE_NO_SIGPIPE_PROTECTION to build without that protection (the application must then ignore SIGPIPE itself)."
#endif

#ifdef MSG_NOSIGNAL
static const int SEND_FLAGS_NO_SIGPIPE = MSG_NOSIGNAL;
#else
static const int SEND_FLAGS_NO_SIGPIPE = 0;
#endif

// Sets SO_NOSIGPIPE on the descriptor, where the platform has that option. Returns false
// only if the sends of a socket on this descriptor are NOT protected as a result: the
// option exists and could not be set, and the sends do not carry MSG_NOSIGNAL either
// (where they do, the option is a second layer and its failure is only logged).
static bool protectDescriptorFromSigpipe(int socketFD) {
#ifdef SO_NOSIGPIPE
   const int on = 1;
   if (0 != ::setsockopt(socketFD, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on))) {
#ifdef MSG_NOSIGNAL
      LOG_WARNING("unable to set SO_NOSIGPIPE (sends use MSG_NOSIGNAL)")
#else
      LOG_ERROR("unable to set SO_NOSIGPIPE")
      return false;
#endif
   }
#else
   (void) socketFD;
#endif
   return true;
}

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
   m_lastReadSize(0),
   m_sigpipeUnprotected(false) {

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
   m_lastReadSize(0),
   m_sigpipeUnprotected(false) {

   LOG_INSTANCE_CREATE("Socket")

   // a handed-over descriptor is not assumed to have been set up for this Socket
   if ((m_socketFD > -1) && !protectDescriptorFromSigpipe(m_socketFD)) {
      m_sigpipeUnprotected = true;
   }
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
   m_lastReadSize(0),
   m_sigpipeUnprotected(false) {

   LOG_INSTANCE_CREATE("Socket")

   // a handed-over descriptor is not assumed to have been set up for this Socket
   if ((m_socketFD > -1) && !protectDescriptorFromSigpipe(m_socketFD)) {
      m_sigpipeUnprotected = true;
   }
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

   // Before connecting: the peer cannot have gone yet, so a failure here is a real error and
   // the construction fails, releasing the descriptor like a failed connect does (the
   // constructor throws, and the destructor of an object whose constructor threw never runs).
   if (!protectDescriptorFromSigpipe(m_socketFD)) {
      const int optionError = errno;
      ::close(m_socketFD);
      m_socketFD = -1;
      errno = optionError;
      return false;
   }

   ::memset((void *) &m_serverAddr, 0, sizeof(m_serverAddr));

   m_serverAddr.sin_family = AF_INET;
   m_serverAddr.sin_port = htons(m_port);
   m_serverAddr.sin_addr.s_addr = inet_addr(m_serverAddress.c_str());

   const int rc = ::connect(m_socketFD,
                            (struct sockaddr *) &m_serverAddr,
                            sizeof(m_serverAddr));

   bool connectFailed = false;
   int connectError = 0;

   if (rc < 0) {
      connectFailed = true;
      connectError = errno;

      if (connectError == EINTR) {
         // A signal handler ran while connect() was waiting. The connection attempt is not
         // over: it goes on in the kernel (on Linux and on macOS, with or without SA_RESTART),
         // so this is neither a failure nor a reason to call connect() again (a second call
         // does not mean the same on every platform). Wait for the attempt to end on the same
         // descriptor and read its outcome. Like connect() itself, the wait has no time limit.
         connectError = detail::awaitConnectCompletion(m_socketFD, detail::connectIoOps());
         connectFailed = (connectError != 0);
      }
   }

   if (connectFailed) {
      // The constructor throws when this returns false, and the destructor of an object
      // whose constructor threw does not run, so this is the only place that can release
      // the descriptor that createSocket() returned. close() may change errno, which the
      // caller may want to inspect, so it is kept.
      ::close(m_socketFD);
      m_socketFD = -1;
      errno = connectError;
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
      // always reset, regardless of m_borrowedDescriptor, so that close()
      // is idempotent -- a later call (e.g. from the destructor) must not
      // close(m_socketFD) again, since the fd number may already have
      // been reused by an unrelated connection by then.
      m_socketFD = -1;
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

bool Socket::setReceiveTimeout(int seconds) {
   if (m_socketFD < 0) {
      return false;
   }

   struct timeval tv;
   tv.tv_sec = seconds;
   tv.tv_usec = 0;

   return (0 == ::setsockopt(m_socketFD,
                             SOL_SOCKET,
                             SO_RCVTIMEO,
                             (char*) &tv,
                             sizeof(tv)));
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

      ssize_t bytesSent;

      // A signal handler that ran while send() waited makes it fail with EINTR when it had
      // transferred nothing; the same two bytes are sent again. Any other result is judged as
      // before.
      do {
         bytesSent = detail::sendIoOps().send(m_socketFD,
                                              &nOrderSize,
                                              sizeof(uint16_t),
                                              SEND_FLAGS_NO_SIGPIPE);
      } while ((bytesSent < 0) && (EINTR == errno));

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
         // peer performed an orderly shutdown -- no more data is coming
         allIsGood = false;
      } else {
         allIsGood = false;
      }
   }

   return totalBytesReceived;
}

//******************************************************************************

bool Socket::readLine(std::string& line) {
   line.erase();

   if (m_includeMessageSize) {
      // one length-prefixed message is the unit of framing here -- it has
      // to be read in a single recvPayload() call (which itself consumes
      // a fresh 2-byte length prefix per call), so a byte-at-a-time scan
      // via readSocket() would desync after the first byte. Read the
      // whole framed message at once and scan the result for a newline.
      char buffer[65535];
      const ssize_t bytesReceived = recvPayload(buffer, sizeof(buffer) - 1, 0);

      if (bytesReceived <= 0) {
         return false;
      }

      ssize_t lineLength = bytesReceived;

      for (ssize_t i = 0; i < bytesReceived; ++i) {
         if (buffer[i] == '\n') {
            lineLength = i;
            break;
         }
      }

      if ((lineLength > 0) && (buffer[lineLength - 1] == '\r')) {
         --lineLength;
      }

      line.assign(buffer, lineLength);
      return true;
   }

   char c;
   bool foundEOL = false;

   while (!foundEOL) {
      const int bytesRead = readSocket(&c, 1);

      if (bytesRead <= 0) {
         return false;
      }

      if (c == '\n') {
         foundEOL = true;
      } else if (c != '\r') {
         line += c;
      }
   }

   return true;

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
   if (nullptr == buffer) {
      return -1;
   }

   int bytesAlreadyRead = 0;
   int remainingBytesToRead = bytesToRead;

   // serve as much as possible from a primed line-input buffer (used by
   // tests to feed a socket without a real connected fd) before touching
   // the real socket
   if (!m_lineInputBuffer.empty()) {
      const int nInputBufferLen = (int) m_lineInputBuffer.length();

      if (nInputBufferLen >= bytesToRead) {
         ::memcpy(buffer, m_lineInputBuffer.c_str(), bytesToRead);
         m_lineInputBuffer.erase(0, bytesToRead);
         return bytesToRead;
      } else {
         ::memcpy(buffer, m_lineInputBuffer.c_str(), nInputBufferLen);
         m_lineInputBuffer.erase();
         bytesAlreadyRead = nInputBufferLen;
         remainingBytesToRead = bytesToRead - nInputBufferLen;
      }
   }

   if ((m_socketFD < 0) || (!m_isConnected)) {
      return (bytesAlreadyRead > 0) ? bytesAlreadyRead : -1;
   }

   const ssize_t bytesFromSocket =
      recvPayload(buffer + bytesAlreadyRead, remainingBytesToRead, 0);

   if (bytesFromSocket <= 0) {
      return (bytesAlreadyRead > 0) ? bytesAlreadyRead : -1;
   }

   return bytesAlreadyRead + (int) bytesFromSocket;

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

int Socket::readSocket(char* buffer, int bytesToRead, int timeoutMillis) {
   // the budget starts here
   const std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();

   if (nullptr == buffer) {
      return -1;
   }

   if (m_includeMessageSize) {
      // see the header: size-prefixed mode cannot be cut short safely
      LOG_ERROR("bounded readSocket is not supported with message size included")
      return -1;
   }

   if (bytesToRead < 0) {
      return -1;
   }

   if (bytesToRead == 0) {
      return 0;
   }

   int bytesAlreadyRead = 0;
   int remainingBytesToRead = bytesToRead;

   // same as readSocket(buffer, bytesToRead): serve a primed line-input
   // buffer first
   if (!m_lineInputBuffer.empty()) {
      const int nInputBufferLen = (int) m_lineInputBuffer.length();

      if (nInputBufferLen >= bytesToRead) {
         ::memcpy(buffer, m_lineInputBuffer.c_str(), bytesToRead);
         m_lineInputBuffer.erase(0, bytesToRead);
         return bytesToRead;
      } else {
         ::memcpy(buffer, m_lineInputBuffer.c_str(), nInputBufferLen);
         m_lineInputBuffer.erase();
         bytesAlreadyRead = nInputBufferLen;
         remainingBytesToRead = bytesToRead - nInputBufferLen;
      }
   }

   if ((m_socketFD < 0) || (!m_isConnected)) {
      return (bytesAlreadyRead > 0) ? bytesAlreadyRead : -1;
   }

   const int bytesFromSocket =
      detail::boundedReadFromDescriptor(m_socketFD,
                                        buffer + bytesAlreadyRead,
                                        remainingBytesToRead,
                                        timeoutMillis,
                                        start,
                                        detail::systemSocketIoOps());

   const int totalBytesRead = bytesAlreadyRead + bytesFromSocket;
   return (totalBytesRead > 0) ? totalBytesRead : -1;
}

//******************************************************************************

int Socket::recvAvailable(char* buffer, int bufferSize) {
   if (nullptr == buffer) {
      return -1;
   }

   // serve from a primed line-input buffer first, same as readSocket()
   if (!m_lineInputBuffer.empty()) {
      const int nInputBufferLen = (int) m_lineInputBuffer.length();
      const int bytesToServe =
         (nInputBufferLen < bufferSize) ? nInputBufferLen : bufferSize;

      ::memcpy(buffer, m_lineInputBuffer.c_str(), bytesToServe);
      m_lineInputBuffer.erase(0, bytesToServe);
      return bytesToServe;
   }

   if ((m_socketFD < 0) || (!m_isConnected)) {
      return -1;
   }

   // a single recv() call -- returns as soon as any data is available,
   // rather than looping (like recvPayload()) until bufferSize bytes
   // have arrived
   const ssize_t bytesReceived = ::recv(m_socketFD, buffer, bufferSize, 0);

   if (bytesReceived <= 0) {
      return -1;
   }

   return (int) bytesReceived;
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
   if (m_sigpipeUnprotected) {
      // a write that the peer's absence could answer with SIGPIPE is refused, not risked
      LOG_ERROR("write refused: SIGPIPE could not be suppressed for this socket")
      return false;
   }

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
      int bytesSent = detail::sendIoOps().send(m_socketFD,
                                               pSendBuffer,
                                               remainingBytes,
                                               flags | SEND_FLAGS_NO_SIGPIPE);
      if (bytesSent > 0) {
         totalBytesSent += bytesSent;
         pSendBuffer += bytesSent;
         remainingBytes -= bytesSent;
      } else if (bytesSent == 0) {
      } else if (EINTR == errno) {
         // A signal handler ran while send() waited and it had transferred nothing in this call
         // (bytes sent by earlier calls are already counted above): go on from the same
         // position with the same remaining length. Not a failure, and nothing is sent twice.
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


//******************************************************************************

namespace {

int systemPoll(struct pollfd* fds, nfds_t nfds, int timeoutMillis) {
   return ::poll(fds, nfds, timeoutMillis);
}

ssize_t systemRecv(int fd, void* buffer, size_t length, int flags) {
   return ::recv(fd, buffer, length, flags);
}

const detail::SocketIoOps SYSTEM_SOCKET_IO_OPS = { systemPoll, systemRecv };

}

//******************************************************************************

const chaudiere::detail::SocketIoOps& chaudiere::detail::systemSocketIoOps() {
   return SYSTEM_SOCKET_IO_OPS;
}

//******************************************************************************

int chaudiere::detail::boundedReadFromDescriptor(int fd,
                                                 char* buffer,
                                                 int bytesToRead,
                                                 int timeoutMillis,
                                                 std::chrono::steady_clock::time_point start,
                                                 const SocketIoOps& ops) {
   typedef std::chrono::steady_clock Clock;

   // only meaningful when timeoutMillis > 0. At most INT_MAX milliseconds, so
   // the addition cannot overflow the clock's representation.
   const Clock::time_point deadline =
      start + std::chrono::milliseconds((timeoutMillis > 0) ? timeoutMillis : 0);

   // true once a pass that made no progress must not be repeated: with no
   // limit the read goes on, with 0 there is nothing more to wait for, and
   // otherwise it depends on whether the budget is used up
   auto mustStop = [&]() -> bool {
      return (timeoutMillis == 0) ||
             ((timeoutMillis > 0) && (Clock::now() >= deadline));
   };

   int totalBytesRead = 0;

   while (totalBytesRead < bytesToRead) {
      // how long poll() may wait this time: the remaining budget, recomputed
      // on every pass so that neither progress nor an interrupted call
      // extends the total
      int waitMillis;
      if (timeoutMillis < 0) {
         waitMillis = -1;
      } else if (timeoutMillis == 0) {
         waitMillis = 0;
      } else {
         const Clock::time_point now = Clock::now();
         if (now >= deadline) {
            waitMillis = 0;
         } else {
            // rounded up: poll() takes whole milliseconds, and rounding down
            // would turn the last fraction of a millisecond into a busy loop
            const std::chrono::milliseconds remaining =
               std::chrono::ceil<std::chrono::milliseconds>(deadline - now);
            waitMillis = (remaining.count() < (long long) timeoutMillis) ?
                         (int) remaining.count() :
                         timeoutMillis;
         }
      }

      struct pollfd pfd;
      pfd.fd = fd;
      pfd.events = POLLIN;
      pfd.revents = 0;

      const int pollResult = ops.poll(&pfd, 1, waitMillis);

      if (pollResult < 0) {
         if ((EINTR == errno) && !mustStop()) {
            // a signal, not a failure: wait again for what remains
            continue;
         }
         break;
      }

      if (pollResult == 0) {
         // nothing became readable within the wait
         if (timeoutMillis < 0) {
            continue;
         }
         if (mustStop()) {
            break;
         }
         // woke slightly early: wait out the rest
         continue;
      }

      // Readable, or an error/hang-up condition. Always go through recv() so
      // that the cause is reported by it: data still queued behind a hang-up
      // is delivered first, then end-of-stream (0) or the error (-1).
      // MSG_DONTWAIT: this recv() cannot block, whatever poll() said.
      const ssize_t bytesReceived = ops.recv(fd,
                                             buffer + totalBytesRead,
                                             (size_t) (bytesToRead - totalBytesRead),
                                             MSG_DONTWAIT);
      const int recvErrno = errno;

      if (bytesReceived > 0) {
         totalBytesRead += (int) bytesReceived;
      } else if (bytesReceived == 0) {
         // orderly shutdown by the peer: no more data is coming
         break;
      } else if ((EINTR == recvErrno) || (EAGAIN == recvErrno) || (EWOULDBLOCK == recvErrno)) {
         // Nothing to take after all (a signal, or readiness that was gone by
         // the time of the call). The budget is checked even though nothing
         // was waited for, so that readiness that keeps being reported
         // without any data cannot run past it.
         if (mustStop()) {
            break;
         }
         if (EINTR != recvErrno) {
            // and a pause, so that such readiness cannot become a busy loop
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
         }
         continue;
      } else {
         break;
      }
   }

   return totalBytesRead;
}

//******************************************************************************

namespace {

// what Socket::open() waits with; the tests replace it (see SocketIo.h)
std::atomic<const detail::SocketIoOps*> g_connectIoOps(&SYSTEM_SOCKET_IO_OPS);

}

//******************************************************************************

namespace {

ssize_t systemSend(int fd, const void* buffer, size_t length, int flags) {
   return ::send(fd, buffer, length, flags);
}

const detail::SendIoOps SYSTEM_SEND_IO_OPS = { systemSend };

// what the send paths transmit with; the tests replace it (see SocketIo.h)
std::atomic<const detail::SendIoOps*> g_sendIoOps(&SYSTEM_SEND_IO_OPS);

}

//******************************************************************************

const chaudiere::detail::SendIoOps& chaudiere::detail::systemSendIoOps() {
   return SYSTEM_SEND_IO_OPS;
}

//******************************************************************************

const chaudiere::detail::SendIoOps& chaudiere::detail::sendIoOps() {
   return *g_sendIoOps.load();
}

//******************************************************************************

const chaudiere::detail::SendIoOps*
chaudiere::detail::setSendIoOps(const SendIoOps* ops) {
   return g_sendIoOps.exchange((ops != nullptr) ? ops : &SYSTEM_SEND_IO_OPS);
}

//******************************************************************************

const chaudiere::detail::SocketIoOps& chaudiere::detail::connectIoOps() {
   return *g_connectIoOps.load();
}

//******************************************************************************

const chaudiere::detail::SocketIoOps*
chaudiere::detail::setConnectIoOps(const SocketIoOps* ops) {
   return g_connectIoOps.exchange((ops != nullptr) ? ops : &SYSTEM_SOCKET_IO_OPS);
}

//******************************************************************************

int chaudiere::detail::awaitConnectCompletion(int fd, const SocketIoOps& ops) {
   // POSIX allows poll() with an infinite timeout to return only for an event or an error. A
   // result that is neither is not expected; it is tolerated a few times, never looped on.
   const int MAX_MEANINGLESS_RESULTS = 8;
   int meaninglessResults = 0;

   for (;;) {
      struct pollfd pfd;
      pfd.fd = fd;
      pfd.events = POLLOUT;
      pfd.revents = 0;

      const int pollResult = ops.poll(&pfd, 1, -1);

      if (pollResult < 0) {
         const int pollError = errno;
         if (EINTR == pollError) {
            // a signal again, not a failure: go on waiting
            continue;
         }
         // the wait itself failed: this is the failure of the connection
         return (pollError != 0) ? pollError : EIO;
      }

      if (pollResult > 0) {
         if (pfd.revents & POLLNVAL) {
            // the descriptor is not open: nothing can be learned from it
            return EBADF;
         }

         // Writable, or an error / hang-up condition. POLLOUT alone does not say that the
         // connection succeeded (a refused attempt is reported as POLLOUT | POLLERR | POLLHUP
         // on Linux), and POLLERR / POLLHUP alone do not say that it failed; only SO_ERROR does.
         // Reading it clears it, so it is read once and the value is returned as it is.
         if (pfd.revents & (POLLOUT | POLLERR | POLLHUP)) {
            int socketError = 0;
            socklen_t length = sizeof(socketError);
            if (0 != ::getsockopt(fd, SOL_SOCKET, SO_ERROR, &socketError, &length)) {
               const int optionError = errno;
               return (optionError != 0) ? optionError : EIO;
            }
            return socketError;
         }
      }

      if (++meaninglessResults > MAX_MEANINGLESS_RESULTS) {
         return ETIMEDOUT;
      }
   }
}
