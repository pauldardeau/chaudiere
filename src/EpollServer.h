// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_EPOLLSERVER_H
#define CHAUDIERE_EPOLLSERVER_H

#include "KernelEventServer.h"


#ifdef __linux__
#include <sys/epoll.h>
#define EPOLL_SUPPORT 1
#endif


namespace chaudiere
{
   class SocketServiceHandler;
   class Mutex;

/**
 * EpollServer is a wrapper for working with the epoll API. The epoll API
 * is a high-performance kernel event mechanism on Linux.
 */
class EpollServer : public KernelEventServer
{
public:
   /**
    * Determines if the Epoll mechanism is supported on the current platform
    * @return boolean indicating if epoll mechanism is supported
    */
   static bool isSupportedPlatform();

   /**
    *
    * @param fdMutex
    * @param hwmConnectionsMutex
    */
   EpollServer(Mutex& fdMutex, Mutex& hwmConnectionsMutex);
   
   /**
    * Destructor
    */
   ~EpollServer();
   
   // KernelEventServer
   /**
    *
    * @param socketServiceHandler
    * @param serverPort
    * @param maxConnections
    * @return
    */
   virtual bool init(SocketServiceHandler* socketServiceHandler,
                     int serverPort,
                     int maxConnections) override;
   
   /**
    *
    * @param maxConnections
    * @return
    */
   virtual int getKernelEvents(int maxConnections) override;
   
   /**
    *
    * @param eventIndex
    * @return
    */
   virtual int fileDescriptorForEventIndex(int eventIndex) override;
   
   /**
    *
    * @param fileDescriptor
    * @return
    */
   virtual bool addFileDescriptorForRead(int fileDescriptor) override;
   
   /**
    *
    * @param fileDescriptor
    * @return
    */
   virtual bool removeFileDescriptorFromRead(int fileDescriptor) override;
   
   /**
    *
    * @param eventIndex
    * @return
    */
   virtual bool isEventDisconnect(int eventIndex) override;
   
   /**
    *
    * @param eventIndex
    * @return
    */
   virtual bool isEventReadClose(int eventIndex) override;

   /**
    *
    * @param eventIndex
    * @return
    */
   virtual bool isEventRead(int eventIndex) override;

   
private:
#ifdef EPOLL_SUPPORT
   struct epoll_event* m_events;
#endif
   int m_epfd;
   
   // copying not allowed
   EpollServer(const EpollServer&);
   EpollServer& operator=(const EpollServer&);

};

}

#endif
