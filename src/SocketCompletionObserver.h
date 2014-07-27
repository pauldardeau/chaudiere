// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_SOCKETCOMPLETIONOBSERVER_H
#define CHAUDIERE_SOCKETCOMPLETIONOBSERVER_H

#include <memory>


namespace chaudiere
{
   class Socket;

/**
 * SocketCompletionObserver is an interface for being notified when a Socket
 * is no longer being used and is being closed.
 */
class SocketCompletionObserver
{
public:
   /**
    * Destructor
    */
   virtual ~SocketCompletionObserver() noexcept {}
   
   /**
    * Notifies the observer that the specified socket is being closed
    * @param socket the socket that is being closed
    */
   virtual void notifySocketComplete(std::shared_ptr<Socket> socket) noexcept = 0;

};

}

#endif
