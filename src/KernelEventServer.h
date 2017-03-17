// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_KERNELEVENTSERVER_H
#define CHAUDIERE_KERNELEVENTSERVER_H

#include <memory>

#include "Socket.h"
#include "SocketCompletionObserver.h"


namespace chaudiere
{
   class Mutex;
   class SocketServiceHandler;

/**
 * KernelEventServer is an abstract base class for kernel event server
 * mechanisms such as kqueue and epoll.
 */
class KernelEventServer : public SocketCompletionObserver
{
public:
   /**
    *
    * @param fdMutex
    * @param hwmConnectionsMutex
    * @param serverName
    */
   KernelEventServer(Mutex& fdMutex,
                     Mutex& hwmConnectionsMutex,
                     const std::string& serverName);
                     
   /**
    * Destructor
    */
   virtual ~KernelEventServer();
   
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
                     int maxConnections);
                     
   /**
    *
    */
   virtual void run();

   /**
    *
    * @param maxConnections
    * @return
    */
   virtual int getKernelEvents(int maxConnections) = 0;

   /**
    *
    * @param eventIndex
    * @return
    */
   virtual int fileDescriptorForEventIndex(int eventIndex) = 0;
   
   /**
    *
    * @param fileDescriptor
    * @return
    */
   virtual bool addFileDescriptorForRead(int fileDescriptor) = 0;
   
   /**
    *
    * @param fileDescriptor
    * @return
    */
   virtual bool removeFileDescriptorFromRead(int fileDescriptor) = 0;

   /**
    *
    * @param eventIndex
    * @return
    */
   virtual bool isEventReadClose(int eventIndex) = 0;
   
   /**
    *
    * @param eventIndex
    * @return
    */
   virtual bool isEventDisconnect(int eventIndex) = 0;
   
   /**
    *
    * @param eventIndex
    * @return
    */
   virtual bool isEventRead(int eventIndex) = 0;
   
   // SocketCompletionObserver
   /**
    *
    * @param socket
    */
   void notifySocketComplete(Socket* socket);

   
protected:
   /**
    *
    * @return
    */
   int getListenerSocketFileDescriptor() const;


private:
   SocketServiceHandler* m_socketServiceHandler;
   bool* m_listBusyFlags;
   int m_serverPort;
   int m_maxConnections;
   int m_listenBacklog;
   int m_listenerFD;
   int m_fdmax;
   int m_numberEventsReturned;

   // copying not allowed
   KernelEventServer(const KernelEventServer&);
   KernelEventServer& operator=(const KernelEventServer&);

};

}

#endif
