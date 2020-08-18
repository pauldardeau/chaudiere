// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_KQUEUESERVER_H
#define CHAUDIERE_KQUEUESERVER_H

#include "KernelEventServer.h"

#if defined(__APPLE__) || \
    defined(__FreeBSD__) || \
    defined(__OpenBSD__) || \
    defined(__NetBSD__)
#define KQUEUE_SUPPORT 1
#include <sys/event.h>
#endif

namespace chaudiere
{
   class SocketServiceHandler;
   class Mutex;

/**
 * KqueueServer is a wrapper for working with the kqueue API. The kqueue API
 * is a high-performance kernel event mechanism on FreeBSD and OSX.
 */
class KqueueServer : public KernelEventServer
{
public:
   /**
    * Determines if the current platform supports the kqueue mechanism
    * @return boolean indicating if kqueue mechanism is supported
    */
   static bool isSupportedPlatform();
   
   /**
    *
    * @param fdMutex
    * @param hwmConnectionsMutex
    */
   KqueueServer(Mutex& fdMutex, Mutex& hwmConnectionsMutex);
   
   /**
    * Destructor
    */
   ~KqueueServer();
   
   // KernelEventServer
   /**
    *
    * @param socketServiceHandler
    * @param serverPort
    * @param maxConnections
    * @return
    * @see SocketServiceHandler()
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
#ifdef KQUEUE_SUPPORT
   struct kevent m_event;
#endif
   int m_kqfd;
   
   // copying not allowed
   KqueueServer(const KqueueServer&);
   KqueueServer& operator=(const KqueueServer&);

};

}

#endif
