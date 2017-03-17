// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_REQUESTHANDLER_H
#define CHAUDIERE_REQUESTHANDLER_H


#include "Runnable.h"


namespace chaudiere
{
   class Socket;
   class SocketRequest;

/**
 *
 */
class RequestHandler : public Runnable
{
private:
   // disallow copies
   RequestHandler(const RequestHandler&);
   RequestHandler& operator=(const RequestHandler&);

   Socket* m_socket;
   SocketRequest* m_socketRequest;
   bool m_isThreadPooling;

   
public:
   /**
    * Constructs a RequestHandler using a SocketRequest for use by a KernelEventServer
    * @param socketRequest the SocketRequest for processing using KernelEventServer
    * @see SocketRequest()
    */
   RequestHandler(SocketRequest* socketRequest);
   
   /**
    * Constructs a RequestHandler using a Socket
    * @param socket the Socket for handling the request
    * @see Socket()
    */
   RequestHandler(Socket* socket);
   
   /**
    * Destructor
    */
   virtual ~RequestHandler();
   
   /**
    * Sets boolean indicating whether request is being run on thread pool
    * @param isThreadPooling boolean indicating if request is being run on thread pool
    */
   void setThreadPooling(bool isThreadPooling);
   
   /**
    *
    * @return
    */
   bool isThreadPooling() const;
   
   /**
    *
    * @return
    * @see Socket()
    */
   Socket* getSocket();
   
};

}

#endif
