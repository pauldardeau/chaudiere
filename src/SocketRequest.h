// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_SOCKETREQUEST_H
#define CHAUDIERE_SOCKETREQUEST_H

#include <memory>
#include "Runnable.h"


namespace chaudiere
{
   class Socket;
   class SocketServiceHandler;

/**
 *
 */
class SocketRequest : public Runnable
{
public:
   /**
    * Constructs a SocketRequest with a Socket and SocketServiceHandler
    * @param socket the Socket associated with the request
    * @param handler the handler to use for processing with the Socket
    * @see Socket()
    * @see SocketServiceHandler()
    */
   SocketRequest(Socket* socket, SocketServiceHandler* handler);
   
   /**
    * Destructor
    */
   ~SocketRequest();
   
   /**
    * Services the socket using the specified handler
    */
   void run();
   
   /**
    * Retrieves the file descriptor for the socket
    * @return socket file descriptor
    */
   int getSocketFD() const;
   
   /**
    * Retrieves the Socket associated with the request
    * @see Socket()
    * @return the Socket associated with the request
    */
   Socket* getSocket();
   
   /**
    * Notifies the Socket that the request processing is complete
    */
   void requestComplete();

   /**
    * @return
    */
   bool isSocketOwned() const;

   /**
    *
    */
   void setSocketOwned(bool socketOwned);

   
private:
   Socket* m_socket;
   SocketServiceHandler* m_handler;
   bool m_socketOwned;

   // copies not allowed
   SocketRequest(const SocketRequest&);
   SocketRequest& operator=(const SocketRequest&);
};

}

#endif
