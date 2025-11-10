// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_SERVERSOCKET_H
#define CHAUDIERE_SERVERSOCKET_H

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <memory>


namespace chaudiere
{
   class Socket;

/**
 * ServerSocket is very similar to Java's ServerSocket class. It provides
 * server-side handling of Socket calls.
 */
class ServerSocket
{
   public:
      /**
       * Turns on the reuse address option on the specified socket
       * @param socketFD the socket file descriptor to change
       * @return boolean indicating whether the update succeeded
       */
      static bool setReuseAddr(int socketFD);

      /**
       * Starts the listening on the specified socket
       * @param socketFD the socket file descriptor to listen
       * @param backlog the backlog value for listening
       * @return boolean indicating whether the listen succeeded
       */
      static bool listen(int socketFD, int backlog);

      /**
       * Binds the socket to the specified port
       * @param socketFD the socket file descriptor to bind
       * @param port the port number value
       * @return boolean indicating whether the bind succeeded
       */
      static bool bind(int socketFD, int port);

      /**
       * Creates a new server socket and starts listening on the specified port
       * @param port the port number to listen on
       * @throw BasicException
       */
      explicit ServerSocket(int port);

      /**
       * Destructor
       */
      ~ServerSocket();

      /**
       * Accepts a new client socket connection
       * @see Socket()
       * @return Socket representing new client connection (caller must delete)
       */
      Socket* accept();

      /**
       * Closes the socket
       */
      void close();



   private:
      // copying not allowed
      ServerSocket(const ServerSocket&);
      ServerSocket& operator=(const ServerSocket&);

      /**
       * Creates the server socket and binds on the specified port
       * @return boolean indicating whether the creation succeeded
       */
      bool create();

      /**
       * Starts listening on the server socket
       * @return boolean indicating whether the listen succeeded
       */
      bool listen();

      int m_serverSocket;
      int m_port;
};

}

#endif


