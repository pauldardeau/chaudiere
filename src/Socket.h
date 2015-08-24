// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_SOCKET_H
#define CHAUDIERE_SOCKET_H

#include <netinet/in.h>
#include <sys/types.h>
#include <string>
#include <memory>


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
   static int createSocket() noexcept;
   
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
   explicit Socket(int socketFD) noexcept;
   
   /**
    *
    * @param completionObserver
    * @param socketFD
    */
   Socket(SocketCompletionObserver* completionObserver, int socketFD) noexcept;
   
   /**
    *
    */
   ~Socket() noexcept;
    
   /**
    *
    * @param sendBuffer
    * @param bufferLength
    * @param flags
    * @return
    */
   ssize_t send(const void* sendBuffer, size_t bufferLength, int flags) noexcept;
   
   /**
    *
    * @param buffer
    * @param bufsize
    * @return
    */
   bool write(const char* buffer, unsigned long bufsize) noexcept;
   
   /**
    *
    * @param payload
    * @return
    */
   bool write(const std::string& payload) noexcept;

    
   /**
    *
    * @param receiveBuffer
    * @param bufferLength
    * @param flags
    * @return
    */
   ssize_t receive(void* receiveBuffer, size_t bufferLength, int flags) noexcept;
   
   /**
    *
    * @param buffer
    * @param bufferLen
    * @return
    */
   bool read(char* buffer, int bufferLen) noexcept;
   
   /**
    *
    * @param buffer
    * @param bytesToRead
    * @return
    */
   bool readSocket(char* buffer, int bytesToRead) noexcept;
   
   /**
    *
    */
   void close() noexcept;
   
   /**
    *
    * @return
    */
   bool isOpen() const noexcept;
   
   /**
    *
    * @return
    */
   bool isConnected() const noexcept;
   
   /**
    *
    */
   void closeConnection() noexcept;
   
   /**
    *
    * @return
    */
   int getFileDescriptor() const noexcept;
   
   /**
    *
    */
   void requestComplete() noexcept;
    
   /**
    *
    * @param userIndex
    */
   void setUserIndex(int userIndex) noexcept;
   
   /**
    *
    * @return
    */
   int getUserIndex() const noexcept;
   
   /**
    *
    * @param on
    * @return
    */
   bool setTcpNoDelay(bool on) noexcept;
   
   /**
    *
    * @return
    */
   bool getTcpNoDelay() const noexcept;
    
   /**
    *
    * @param size
    * @return
    */
   bool setSendBufferSize(int size) noexcept;
   
   /**
    *
    * @return
    */
   int getSendBufferSize() const noexcept;
    
   /**
    *
    * @param size
    * @return
    */
   bool setReceiveBufferSize(int size) noexcept;
   
   /**
    *
    * @return
    */
   int getReceiveBufferSize() const noexcept;
    
   /**
    *
    * @param on
    * @return
    */
   bool setKeepAlive(bool on) noexcept;
   
   /**
    *
    * @return
    */
   bool getKeepAlive() const noexcept;
    
   /**
    *
    * @param line
    * @return
    */
   bool readLine(std::string& line) noexcept;
    
   /**
    *
    * @param ipAddress
    * @return
    */
   bool getPeerIPAddress(std::string& ipAddress) noexcept;
    
   /**
    *
    * @return
    */
   int getPort() const noexcept;
    
   /**
    *
    * @param isSizeIncluded
    */
   void setIncludeMessageSize(bool isSizeIncluded) noexcept;

   
protected:
   bool readMsg(int length) noexcept;
   //bool readSocket(char* buffer, int bytesToRead) noexcept;
   bool open() noexcept;
   void init() noexcept;
   
   void setLineInputBuffer(const std::string& s) noexcept;
   void appendLineInputBuffer(const std::string& s) noexcept;

    
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
   char* m_inputBuffer; // allocated with "new char[]"
   int m_inBufferSize;
   int m_lastReadSize;
};

}

#endif
