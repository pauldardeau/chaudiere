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
                     const std::string& serverName) noexcept;
                     
   /**
    * Destructor
    */
   virtual ~KernelEventServer() noexcept;
   
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
                     int maxConnections) noexcept;
                     
   /**
    *
    */
   virtual void run() noexcept;

   /**
    *
    * @param maxConnections
    * @return
    */
   virtual int getKernelEvents(int maxConnections) noexcept = 0;

   /**
    *
    * @param eventIndex
    * @return
    */
   virtual int fileDescriptorForEventIndex(int eventIndex) noexcept = 0;
   
   /**
    *
    * @param fileDescriptor
    * @return
    */
   virtual bool addFileDescriptorForRead(int fileDescriptor) noexcept = 0;
   
   /**
    *
    * @param fileDescriptor
    * @return
    */
   virtual bool removeFileDescriptorFromRead(int fileDescriptor) noexcept = 0;

   /**
    *
    * @param eventIndex
    * @return
    */
   virtual bool isEventReadClose(int eventIndex) noexcept = 0;
   
   /**
    *
    * @param eventIndex
    * @return
    */
   virtual bool isEventDisconnect(int eventIndex) noexcept = 0;
   
   /**
    *
    * @param eventIndex
    * @return
    */
   virtual bool isEventRead(int eventIndex) noexcept = 0;
   
   // SocketCompletionObserver
   /**
    *
    * @param socket
    */
   void notifySocketComplete(Socket* socket) noexcept override;

   // copying not allowed
   KernelEventServer(const KernelEventServer&) = delete;
   KernelEventServer& operator=(const KernelEventServer&) = delete;

   
protected:
   /**
    *
    * @return
    */
   int getListenerSocketFileDescriptor() const noexcept;


private:
   SocketServiceHandler* m_socketServiceHandler;
   bool* m_listBusyFlags;
   int m_serverPort;
   int m_maxConnections;
   int m_listenBacklog;
   int m_listenerFD;
   int m_fdmax;
   int m_numberEventsReturned;

};

}

#endif
