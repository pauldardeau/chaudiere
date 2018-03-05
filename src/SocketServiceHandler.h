// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_SOCKETSERVICEHANDLER_H
#define CHAUDIERE_SOCKETSERVICEHANDLER_H

#include <string>


namespace chaudiere
{
   class SocketRequest;

/**
 * SocketServiceHandler is an interface for a handler that processes a SocketRequest
 */
class SocketServiceHandler
{
public:
   /**
    * Destructor
    */
   virtual ~SocketServiceHandler() {}
   
   /**
    * Process a SocketRequest
    * @param socketRequest the SocketRequest to process
    * @see SocketRequest()
    */
   virtual void serviceSocket(SocketRequest* socketRequest) = 0;
   
   /**
    * Retrieves the name of the handler. This is primarily an aid for debugging.
    * @return the name of the handler
    */
   virtual const std::string& getName() const = 0;
};

}

#endif
