// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <string>

#include "KernelEventServer.h"
#include "Socket.h"
#include "SocketServiceHandler.h"
#include "SocketRequest.h"
#include "MutexLock.h"
#include "Logger.h"
#include "ServerSocket.h"
#include "BasicException.h"
#include "ThreadingFactory.h"

using namespace std;
using namespace chaudiere;

//******************************************************************************

KernelEventServer::KernelEventServer(Mutex& fdMutex,
                                     Mutex& hwmConnectionsMutex,
                                     const std::string& serverName) :
   m_socketServiceHandler(nullptr),
   m_serverPort(0),
   m_maxConnections(0),
   m_listenBacklog(10),
   m_listenerFD(-1),
   m_numberEventsReturned(0) {
}

//******************************************************************************

KernelEventServer::~KernelEventServer() {
   
   delete m_socketServiceHandler;
   
   if (-1 != m_listenerFD) {
      ::close(m_listenerFD);
   }
}

//******************************************************************************

bool KernelEventServer::init(SocketServiceHandler* socketServiceHandler,
                             int serverPort,
                             int maxConnections) {
   m_socketServiceHandler = socketServiceHandler;
   m_serverPort = serverPort;
   m_maxConnections = maxConnections;
   
   if (nullptr == m_socketServiceHandler) {
      Logger::critical("no socket service handler set");
      return false;
   }
   
   if (m_serverPort <= 0) {
      Logger::critical("serverPort must be positive, non-zero value");
      return false;
   }
   
   if (m_maxConnections <= 0) {
      Logger::critical("maxConnections must be positive, non-zero value");
      return false;
   }

   ThreadingFactory* tf = ThreadingFactory::getThreadingFactory();
   m_busyFlagsMutex = tf->createMutex("busyFlags");
   
   m_listenerFD = Socket::createSocket();
   if (m_listenerFD == -1) {
      Logger::critical("error: unable to create server listening socket");
      return false;
   }
   
   if (!ServerSocket::setReuseAddr(m_listenerFD)) {
      Logger::critical("unable to set REUSEADDR for socket");
      return false;
   }
   
   if (!ServerSocket::bind(m_listenerFD, m_serverPort)) {
      Logger::critical("bind failed");
      return false;
   }
   
   if (!ServerSocket::listen(m_listenerFD, m_listenBacklog)) {
      Logger::critical("listen failed");
      return false;
   }
   
   return true;
}

//******************************************************************************

bool KernelEventServer::isValidDescriptor(int fd) const {
#ifdef __linux__
   return ::fcntl(fd, F_GETFD) != EBADF;
#else
   //TODO: put non-linux test of descriptor
   return true;
#endif
}

//******************************************************************************

void KernelEventServer::run() {
   struct sockaddr_in clientaddr;
   socklen_t addrlen = sizeof(clientaddr);
   int newfd;
   //char msg[128];
   
   const std::string& handlerName = m_socketServiceHandler->getName();
   
   Logger::info(std::string("using handler: ") + handlerName);
   
   for (;;) {
      
      m_numberEventsReturned = getKernelEvents(m_maxConnections);
      
      if (m_numberEventsReturned < 1) {
         continue;
      }
      
      for (int index = 0; index < m_numberEventsReturned; ++index) {
         
         const int client_fd = fileDescriptorForEventIndex(index);

         if (client_fd == m_listenerFD) {
            newfd = ::accept(m_listenerFD, (struct sockaddr *)&clientaddr, &addrlen);
            if (newfd == -1) {
               Logger::warning("server accept failed");
            } else {
               if (!addFileDescriptorForRead(newfd)) {
                  Logger::critical("kernel event server failed adding read filter");
               }
            }
         } else {
            if (client_fd == 0) {
               continue;
            }

            if (!isValidDescriptor(client_fd)) {
               removeBusyFD(client_fd);
               removeFileDescriptorFromRead(client_fd);
               continue;
            }

            if (isEventReadClose(index)) {
               removeBusyFD(client_fd);
               if (!removeFileDescriptorFromRead(client_fd)) {
                  Logger::warning("kernel event server failed to delete read filter");
               }
               ::close(client_fd);
            } else if (isEventDisconnect(index)) {
               removeBusyFD(client_fd);
               if (!removeFileDescriptorFromRead(client_fd)) {
                  Logger::warning("kernel event server failed to delete read filter");
               }
               ::close(client_fd);
            } else if (isEventRead(index)) {
               if (removeFileDescriptorFromRead(client_fd)) {
                  // are we already busy with this socket?
                  const bool isAlreadyBusy = isBusyFD(client_fd);
               
                  if (!isAlreadyBusy) {
                     //if (!removeFileDescriptorFromRead(client_fd)) {
                     //   Logger::error("unable to remove file descriptor from read");
                     //}
                 
                     setBusyFD(client_fd, true);
                  
                     SocketRequest* socketRequest =
                        new SocketRequest(this, client_fd, nullptr);
                     socketRequest->setSocketOwned(false);
                     socketRequest->setUserIndex(index);
                     socketRequest->setAutoDelete();

                     try {
                        m_socketServiceHandler->serviceSocket(socketRequest);
                     } catch (const BasicException& be) {
                        Logger::error("exception in serviceSocket on handler: " + be.whatString());
                     } catch (const std::exception& e) {
                        Logger::error("exception in serviceSocket on handler: " + std::string(e.what()));
                     } catch (...) {
                        Logger::error("exception in serviceSocket on handler");
                     }
                  } else {
                     //::snprintf(msg, 128, "already busy with socket %d", client_fd);
                     //Logger::warning(msg);
                  }
               } else {
                  removeBusyFD(client_fd);
               }

            }
         }
      }
   }  // for (;;)
}

//******************************************************************************

void KernelEventServer::notifySocketComplete(Socket* socket) {
   const int socketFD = socket->getFileDescriptor();
   if (socketFD == -1) {
      return;
   }
  
   // mark the fd as not being busy anymore
   setBusyFD(socketFD, false);
   
   if (!isValidDescriptor(socketFD)) {
      //if (!removeFileDescriptorFromRead(socketFD)) {
      //   Logger::error("unable to remove file descriptor from read");
      //}
      removeBusyFD(socketFD);
   } else {
      // add socket back to watch
      if (socket->isOpen()) {
         if (!addFileDescriptorForRead(socketFD)) {
            Logger::critical("kernel event add read filter failed");
         }
      } else {
         removeBusyFD(socketFD);
      }
   }
}

//******************************************************************************

int KernelEventServer::getListenerSocketFileDescriptor() const {
   return m_listenerFD;
}

//******************************************************************************

bool KernelEventServer::isBusyFD(int fd) const {
   MutexLock locker(*m_busyFlagsMutex);
   unordered_map<int,bool>::const_iterator it = m_busyFlags.find(fd);
   if (it != m_busyFlags.end()) {
      return it->second;
   } else {
      return false;
   }
}

//******************************************************************************

void KernelEventServer::setBusyFD(int fd, bool busy) {
   MutexLock locker(*m_busyFlagsMutex);
   unordered_map<int,bool>::iterator it = m_busyFlags.find(fd);
   if (it != m_busyFlags.end()) {
      it->second = busy;
   } else {
      m_busyFlags[fd] = busy;
   }
}

//******************************************************************************

bool KernelEventServer::removeBusyFD(int fd) {
   MutexLock locker(*m_busyFlagsMutex);
   unordered_map<int,bool>::iterator it = m_busyFlags.find(fd);
   if (it != m_busyFlags.end()) {
      m_busyFlags.erase(it);
      return true;
   } else {
      return false;
   }
}

//******************************************************************************

