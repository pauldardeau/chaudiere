// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_SOCKET_H
#define CHAUDIERE_SOCKET_H

#include <netinet/in.h>
#include <sys/types.h>
#include <string>
#include <memory>

#include "CharBuffer.h"


namespace chaudiere
{
   class SocketCompletionObserver;

/**
 * Socket is very similar to Java's Socket class. It provides a wrapper class
 * for working with sockets.
 */
class Socket
{
public:
   /**
    *
    * @return
    */
   static int createSocket();

   /**
    *
    * @param address
    * @param port
    * @throws BasicException
    */
   Socket(const std::string& address, int port);

   /**
    *
    * @param socketFD
    */
   explicit Socket(int socketFD);

   /**
    *
    * @param completionObserver
    * @param socketFD
    */
   Socket(SocketCompletionObserver* completionObserver, int socketFD);

   /**
    *
    */
   ~Socket();

   /**
    *
    * @param sendBuffer
    * @param bufferLength
    * @param flags
    * @return
    */
   ssize_t send(const void* sendBuffer, size_t bufferLength, int flags);

   /**
    *
    * @param buffer
    * @param bufsize
    * @return
    */
   bool write(const char* buffer, unsigned long bufsize);

   /**
    *
    * @param payload
    * @return
    */
   bool write(const std::string& payload);


   /**
    *
    * @param receiveBuffer
    * @param bufferLength
    * @param flags
    * @return
    */
   ssize_t receive(void* receiveBuffer, size_t bufferLength, int flags);

   /**
    *
    * @param buffer
    * @param bufferLen
    * @return
    */
   bool read(char* buffer, int bufferLen);

   /**
    *
    * @param buffer
    * @param bytesToRead
    * @return
    */
   int readSocket(char* buffer, int bytesToRead);

   /**
    *
    */
   void close();

   /**
    *
    * @return
    */
   bool isOpen() const;

   /**
    *
    * @return
    */
   bool isConnected() const;

   /**
    *
    */
   void closeConnection();

   /**
    *
    * @return
    */
   int getFileDescriptor() const;

   /**
    *
    */
   void requestComplete();

   /**
    *
    * @param userIndex
    */
   void setUserIndex(int userIndex);

   /**
    *
    * @return
    */
   int getUserIndex() const;

   /**
    *
    * @param on
    * @return
    */
   bool setTcpNoDelay(bool on);

   /**
    *
    * @return
    */
   bool getTcpNoDelay() const;

   /**
    *
    * @param size
    * @return
    */
   bool setSendBufferSize(int size);

   /**
    *
    * @return
    */
   int getSendBufferSize() const;

   /**
    *
    * @param size
    * @return
    */
   bool setReceiveBufferSize(int size);

   /**
    *
    * @return
    */
   int getReceiveBufferSize() const;

   /**
    *
    * @param on
    * @return
    */
   bool setKeepAlive(bool on);

   /**
    *
    * @return
    */
   bool getKeepAlive() const;

   /**
    *
    * @param line
    * @return
    */
   bool readLine(std::string& line);

   /**
    *
    * @param ipAddress
    * @return
    */
   bool getPeerIPAddress(std::string& ipAddress);

   /**
    *
    * @return
    */
   int getPort() const;

   /**
    *
    * @param isSizeIncluded
    */
   void setIncludeMessageSize(bool isSizeIncluded);

   /**
    *
    * @return
    */
   bool isDescriptorBorrowed() const;

protected:
   bool readMsg(int length);
   //bool readSocket(char* buffer, int bytesToRead);
   bool open();
   void init();

   void setLineInputBuffer(const std::string& s);
   void appendLineInputBuffer(const std::string& s);


private:
   // copying not allowed
   Socket(const Socket&);
   Socket& operator=(const Socket&);

   SocketCompletionObserver* m_completionObserver;
   std::string m_lineInputBuffer;
   std::string m_serverAddress;
   struct sockaddr_in m_serverAddr;
   int m_socketFD;
   int m_userIndex;
   int m_port;
   bool m_isConnected;
   bool m_includeMessageSize;
   bool m_borrowedDescriptor;
   CharBuffer m_inputBuffer;
   int m_inBufferSize;
   int m_lastReadSize;
};

}

#endif
