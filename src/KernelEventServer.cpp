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

#include <string>

#include "KernelEventServer.h"
#include "Socket.h"
#include "SocketServiceHandler.h"
#include "SocketRequest.h"
#include "MutexLock.h"
#include "Logger.h"
#include "ServerSocket.h"
#include "BasicException.h"

using namespace std;
using namespace chaudiere;

//******************************************************************************

KernelEventServer::KernelEventServer(Mutex& fdMutex,
                                     Mutex& hwmConnectionsMutex,
                                     const std::string& serverName) :
   m_socketServiceHandler(nullptr),
   m_listBusyFlags(NULL),
   m_serverPort(0),
   m_maxConnections(0),
   m_listenBacklog(10),
   m_listenerFD(-1),
   m_fdmax(0),
   m_numberEventsReturned(0) {
}

//******************************************************************************

KernelEventServer::~KernelEventServer() {
   if (NULL != m_listBusyFlags) {
      ::free(m_listBusyFlags);
      m_listBusyFlags = NULL;
   }
   
   if (-1 != m_listenerFD) {
      ::close(m_listenerFD);
   }
}

//******************************************************************************

bool KernelEventServer::init(unique_ptr<SocketServiceHandler>& socketServiceHandler,
                             int serverPort,
                             int maxConnections) {
   m_socketServiceHandler = std::move(socketServiceHandler);
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
   
   m_fdmax = m_listenerFD; // so far, it's this one
   m_listBusyFlags = (bool*) ::calloc(m_maxConnections, sizeof(bool));
   
   return true;
}

//******************************************************************************

void KernelEventServer::run() {
   struct sockaddr_in clientaddr;
   socklen_t addrlen = sizeof(clientaddr);
   int newfd;
   char msg[128];
   
   const std::string& handlerName = m_socketServiceHandler->getName();
   
   Logger::info(std::string("using handler: ") + handlerName);
   
   for (;;) {
      
      const bool isLoggingDebug = Logger::isLogging(Debug);
      
      m_numberEventsReturned = getKernelEvents(m_maxConnections);
      
      if (isLoggingDebug) {
         ::snprintf(msg, 128, "KernelEventServer::run, numberEventsReturned = %d",
                  m_numberEventsReturned);
         Logger::debug(msg);
      }
      
      if (m_numberEventsReturned < 1) {
         continue;
      }
      
      for (int index = 0; index < m_numberEventsReturned; ++index) {
         
         int client_fd = fileDescriptorForEventIndex(index);
         
         if (isLoggingDebug) {
            ::snprintf(msg, 128, "KernelEventServer::run waiting for locks fd=%d",
                     client_fd);
            Logger::debug(msg);
         }
         
         if (isLoggingDebug) {
            Logger::debug("KernelEventServer::run have locks");
         }
         
         if (client_fd == m_listenerFD) {
            newfd = ::accept(m_listenerFD, (struct sockaddr *)&clientaddr, &addrlen);
            if (newfd == -1) {
               Logger::warning("server accept failed");
            } else {
               if (isLoggingDebug) {
                  ::snprintf(msg, 128, "client %d connected", newfd);
                  Logger::debug(msg);
               }
               
               if (!addFileDescriptorForRead(newfd)) {
                  Logger::critical("kernel event server failed adding read filter");
               }
            }
         } else {
            if (isEventReadClose(index)) {
               if (isLoggingDebug) {
                  ::snprintf(msg, 128, "client closed read %d", client_fd);
                  Logger::debug(msg);
               }
               
               // was it busy?
               if (m_listBusyFlags[index]) {
                  m_listBusyFlags[index] = false;
               }
               
               if (!removeFileDescriptorFromRead(client_fd)) {
                  Logger::warning("kernel event server failed to delete read filter");
               }
            }
            else if (isEventDisconnect(index)) {
               if (isLoggingDebug) {
                  ::snprintf(msg, 128, "client disconnected %d", client_fd);
                  Logger::debug(msg);
               }
               
               // was it busy?
               if (m_listBusyFlags[index]) {
                  m_listBusyFlags[index] = false;
               }
               
               if (!removeFileDescriptorFromRead(client_fd)) {
                  Logger::warning("kernel event server failed to delete read filter");
               }
               
               ::close(client_fd);
            } else if (isEventRead(index)) {
               
               // are we already busy with this socket?
               const bool isAlreadyBusy = m_listBusyFlags[index];
               
               if (!isAlreadyBusy) {
                  if (isLoggingDebug) {
                     ::snprintf(msg, 128, "handling read for socket %d", client_fd);
                     Logger::debug(msg);
                  }
                  
                  // remove file descriptor from watch
                  if (isLoggingDebug) {
                     ::snprintf(msg, 128, "removing socket from watch for read (%d)",
                              client_fd);
                     Logger::debug(msg);
                  }

                  if (!removeFileDescriptorFromRead(client_fd)) {
                     Logger::error("unable to remove file descriptor from read");
                  }
                  
                  m_listBusyFlags[index] = true;
                  
                  //TODO: grab stats (connections, requests)
                  
                  // give up our lock
                  if (isLoggingDebug) {
                     ::snprintf(msg, 128, "giving up locks and dispatching request for socket %d",
                              client_fd);
                     Logger::debug(msg);
                  }
                  
                  Socket clientSocket(this, client_fd);
                  clientSocket.setUserIndex(index);

                  SocketRequest socketRequest(&clientSocket,
                                              m_socketServiceHandler.get());

                  try {
                     m_socketServiceHandler->serviceSocket(&socketRequest);
                  } catch (const BasicException& be) {
                     Logger::error("exception in serviceSocket on handler: " + be.whatString());
                  } catch (const std::exception& e) {
                     Logger::error("exception in serviceSocket on handler: " + std::string(e.what()));
                  } catch (...) {
                     Logger::error("exception in serviceSocket on handler");
                  }
                  
                  m_listBusyFlags[index] = false;
                  addFileDescriptorForRead(client_fd);
                  
               } else {
                  ::snprintf(msg, 128, "already busy with socket %d", client_fd);
                  Logger::warning(msg);
               }
            }
         }
         
         if (isLoggingDebug) {
            Logger::debug("KernelEventServer::run finished iteration of inner loop");
         }
      }
   }  // for (;;)
}

//******************************************************************************

void KernelEventServer::notifySocketComplete(Socket* socket) {
   char msg[128];
   const bool isLoggingDebug = Logger::isLogging(Debug);
   
   const int socketFD = socket->getFileDescriptor();
   
   if (isLoggingDebug) {
      ::snprintf(msg, 128, "completed request with socket %d", socketFD);
      Logger::debug(msg);
   }
   
   const int userIndex = socket->getUserIndex();
   
   if (isLoggingDebug) {
      Logger::debug("notifySocketComplete: waiting for locks");
   }
   
   if (isLoggingDebug) {
      Logger::debug("notifySocketComplete: have locks");
   }
   
   // mark the fd as not being busy anymore
   m_listBusyFlags[userIndex] = false;
   
   if (!socket->isConnected()) {
      
      // remove file descriptor and close socket
      if (isLoggingDebug) {
         ::snprintf(msg, 128, "notifySocketComplete: client disconnect %d",
                  socketFD);
         Logger::debug(msg);

         ::snprintf(msg, 128, "removing file descriptor %d", socketFD);
         Logger::debug(msg);
      }
      
      if (!removeFileDescriptorFromRead(socketFD)) {
         Logger::error("unable to remove file descriptor from read");
      }
      
      ::close(socketFD);
   } else {
      // it's still connected
      
      if (isLoggingDebug) {
         ::snprintf(msg, 128, "adding socket back to watch for read (%d)",
                  socketFD);
         Logger::debug(msg);
      }

      // add socket back to watch
      if (!addFileDescriptorForRead(socketFD)) {
         Logger::critical("kernel event add read filter failed");
      }
   }
}

//******************************************************************************

int KernelEventServer::getListenerSocketFileDescriptor() const {
   return m_listenerFD;
}

//******************************************************************************

