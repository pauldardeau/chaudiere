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
    * Create a new socket and returns the file descriptor
    * @return the file descriptor created or -1 on error
    */
   static int createSocket();

   /**
    * Socket constructor with hostname/IP address and port number
    * @param address hostname or IP address of peer
    * @param port the port number to connect with
    * @throws BasicException
    */
   Socket(const std::string& address, int port);

   /**
    * Socket constructor with existing socket file descriptor
    * @param socketFD the file descriptor to use with the new Socket
    */
   explicit Socket(int socketFD);

   /**
    * Socket constructor with completion observer and existing socket file descriptor
    * @param completionObserver the completion observer to call on 'requestComplete'
    * @param socketFD the file descriptor to use with the new socket
    */
   Socket(SocketCompletionObserver* completionObserver, int socketFD);

   /**
    * Socket destructor
    */
   ~Socket();

   /**
    * Low-level send of buffer, length, and flags to socket connection
    * @param sendBuffer the buffer to send from
    * @param bufferLength the size of the buffer
    * @param flags the flags to use
    * @return number of bytes sent or -1 on error
    */
   ssize_t send(const char* sendBuffer, size_t bufferLength, int flags);

   /**
    * Writes the specified buffer and it's size to the socket
    * @param buffer the buffer to write from
    * @param bufsize the size of the buffer
    * @return boolean indicating whether the write succeeded
    */
   bool write(const char* buffer, unsigned long bufsize);

   /**
    * Writes the specified string to the socket
    * @param payload the string to write
    * @return boolean indicating whether the write succeeded
    */
   bool write(const std::string& payload);

   /**
    * Low-level receive of data from socket into specified buffer and with specified flags
    * @param receiveBuffer the buffer to receive the data read
    * @param bufferLength size of the buffer
    * @param flags the flags to use for the read
    * @return the number of bytes read or -1 on error
    */
   ssize_t receive(char* receiveBuffer, ssize_t bufferLength, int flags);

   /**
    * Reads from the socket into the designated buffer up to the length of the buffer
    * @param buffer the buffer to contain the read bytes
    * @param bufferLen the size of the buffer
    * @return boolean indicating whether the read was successful
    */
   bool read(char* buffer, int bufferLen);

   /**
    * Reads from the socket into the designated buffer up to the specified number of bytes
    * @param buffer the buffer to contain the read bytes
    * @param bytesToRead the maxinum number of bytes to read
    * @return integer specifying number of bytes read (-1 on error)
    */
   int readSocket(char* buffer, int bytesToRead);

   /**
    * Read a string up to (and including) a new-line (\n) character
    * @param line variable to receive the string read
    * @return boolean indicating whether the read succeeded
    */
   bool readLine(std::string& line);

   /**
    * Close the socket
    */
   void close();

   /**
    * Tests whether the socket has a connection
    * @return boolean indicating whether there is a connection
    */
   bool isConnected() const;

   /**
    * Closes the connection (if any)
    */
   void closeConnection();

   /**
    * Returns the file descriptor for the socket
    * @return the file descriptor or -1 if one has not been set or if the connection has been closed
    */
   int getFileDescriptor() const;

   /**
    * A signalling method to indicate that the necessary processing is complete. Calling this
    * method will trigger a call to the completion observer if one has been set.
    */
   void requestComplete();

   /**
    * Sets the user index for the socket
    * @param userIndex the user index to set
    */
   void setUserIndex(int userIndex);

   /**
    * Retrieves the user index for the socket
    * @return the user index or -1 if one has not been set
    */
   int getUserIndex() const;

   /**
    * Sets TCP no-delay flag on or off
    * @param on whether the flag should be turned on
    * @return boolean indicating whether the setting was made
    */
   bool setTcpNoDelay(bool on);

   /**
    * Retrieves the current setting for TCP no-delay flag
    * @return boolean indicating the current value for the flag
    */
   bool getTcpNoDelay() const;

   /**
    * Sets the send buffer size
    * @param size the new size for the send buffer
    * @return boolean indicating whether the setting was made successfully
    */
   bool setSendBufferSize(int size);

   /**
    * Retrieves the current send buffer size
    * @return the current size for the send buffer
    */
   int getSendBufferSize() const;

   /**
    * Sets the receive buffer size
    * @param size the new size for the receive buffer
    * @return boolean indicating whether the setting was made successfully
    */
   bool setReceiveBufferSize(int size);

   /**
    * Retrieves the current receive buffer size
    * @return the current size for the receive buffer
    */
   int getReceiveBufferSize() const;

   /**
    * Sets the keep-alive flag on or off
    * @param on whether the flag should be turned on
    * @return boolean indicating whether the flag was updated
    */
   bool setKeepAlive(bool on);

   /**
    * Retrieves the current value for the keep-alive flag
    * @return boolean indicating the current value
    */
   bool getKeepAlive() const;

   /**
    * Retrieves the IP address of the connected peer
    * @param ipAddress variable to receive the IP address
    * @return boolean indicating whether the peer IP address was retrieved
    */
   bool getPeerIPAddress(std::string& ipAddress);

   /**
    * Retrieve the port number used by the socket
    * @return port number
    */
   int getPort() const;

   /**
    * Sets boolean flag to indicate whether the message size should be sent ahead of the message
    * @param isSizeIncluded whether the size is sent or not
    */
   void setIncludeMessageSize(bool isSizeIncluded);

   /**
    * Retrieves flag indicating whether message size is sent to peer along with each payload
    * @return boolean indicating whether message size is also sent to peer
    */
   bool getIncludeMessageSize() const;

   /**
    * Determine whether the socket descriptor is borrowed
    * @return boolean indicating if the descriptor is borrowed
    */
   bool isDescriptorBorrowed() const;

protected:
   /**
    * Reads data of the specified length into internal buffer
    * @param length the size of data (in bytes) to read
    * @return boolean indicating whether the read succeeded
    */
   bool readMsg(int length);

   /**
    * Opens the socket connection with the designated peer
    * @return boolean indicating whether the connection was successfully made
    */
   bool open();

   /**
    * Sets default settings for new Socket instance
    */
   void init();

   /**
    * Populate the line input buffer with the specified string
    * @param s the string to populate line input buffer
    */
   void setLineInputBuffer(const std::string& s);

   /**
    * Appends the specified string to the line input buffer
    * @param s the string to append to the line input buffer
    */
   void appendLineInputBuffer(const std::string& s);

   bool sendPayloadSize(uint16_t payloadSize);
   bool recvPayloadSize(uint16_t& payloadSize);
   bool sendPayload(const char* buffer, ssize_t payloadSize, int flags);
   ssize_t recvPayload(char* buffer, ssize_t bufferSize, int flags);

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
