// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <errno.h>

#include "EpollServer.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

bool EpollServer::isSupportedPlatform() {
#ifdef EPOLL_SUPPORT
   return true;
#else
   return false;
#endif
}

//******************************************************************************

EpollServer::EpollServer(Mutex& fdMutex, Mutex& hwmConnectionsMutex) :
   KernelEventServer(fdMutex, hwmConnectionsMutex, "EpollServer"),
#ifdef EPOLL_SUPPORT
   m_events(NULL),
#endif
   m_epfd(-1) {
   Logger::logInstanceCreate("EpollServer");
}

//******************************************************************************

EpollServer::~EpollServer() {
   Logger::logInstanceDestroy("EpollServer");

#ifdef EPOLL_SUPPORT
   if (NULL != m_events) {
      ::free(m_events);
      m_events = NULL;
   }
#endif
   
   if (-1 != m_epfd) {
      ::close(m_epfd);
   }
}

//******************************************************************************

bool EpollServer::init(SocketServiceHandler* socketServiceHandler,
                       int serverPort,
                       int maxConnections) {
#ifndef EPOLL_SUPPORT
   return false;
#endif

#ifdef EPOLL_SUPPORT
   if (m_events) {
      ::free(m_events);
      m_events = NULL;
   }
   
   if (KernelEventServer::init(socketServiceHandler, serverPort, maxConnections)) {
      m_epfd = ::epoll_create(maxConnections);
      
      if (m_epfd == -1) {
         Logger::critical("epoll create failed");
         return false;
      }
      
      m_events = (struct epoll_event*) ::calloc(maxConnections,
                                                sizeof(struct epoll_event));
      
      // add our listener socket (server socket) as one of the fd's that
      // we want watched
      if (!addFileDescriptorForRead(getListenerSocketFileDescriptor())) {
         Logger::critical("unable to add listener socket for read");
         return false;
      } else {
         return true;
      }
   }
#endif

   return false;
}

//******************************************************************************

int EpollServer::getKernelEvents(int maxConnections) {
#ifdef EPOLL_SUPPORT
   ::epoll_wait(m_epfd, m_events, maxConnections, -1);
   return maxConnections;
#else
   return 0;
#endif
}

//******************************************************************************

int EpollServer::fileDescriptorForEventIndex(int eventIndex) {
   int client_fd = -1;
   
#ifdef EPOLL_SUPPORT
   struct epoll_event current_event;
   current_event = m_events[eventIndex];
   client_fd = (int) current_event.data.fd;
#endif
   
   return client_fd;
}

//******************************************************************************

bool EpollServer::addFileDescriptorForRead(int fileDescriptor) {
#ifdef EPOLL_SUPPORT
   struct epoll_event ev;
   ::memset(&ev, 0, sizeof(struct epoll_event));
   ev.events = EPOLLIN | EPOLLRDHUP;
   ev.data.fd = fileDescriptor;
   
   if (::epoll_ctl(m_epfd, EPOLL_CTL_ADD, fileDescriptor, &ev) < 0) {
      Logger::critical("epoll_ctl failed in add filter");
      if (errno == EBADF) {
         printf("EBADF\n");
      } else if (errno == EEXIST) {
         printf("EEXIST\n");
      } else if (errno == EINVAL) {
         printf("EINVAL\n");
      } else if (errno == ENOMEM) {
         printf("ENOMEM\n");
      } else if (errno == ENOSPC) {
         printf("ENOSPC\n");
      } else if (errno == EPERM) {
         printf("EPERM\n");
      } else {
         printf("unrecognized error for EPOLL_CTL_ADD\n");
      }
      return false;
   } else {
      return true;
   }
#endif
   
   return false;
}

//******************************************************************************

bool EpollServer::removeFileDescriptorFromRead(int fileDescriptor) {
#ifdef EPOLL_SUPPORT
   struct epoll_event ev;
   ::memset(&ev, 0, sizeof(struct epoll_event));

   if (::epoll_ctl(m_epfd, EPOLL_CTL_DEL, fileDescriptor, &ev) < 0) {
      Logger::critical("epoll_ctl failed in delete filter");
      if (errno == EBADF) {
         printf("EBADF\n");
      } else if (errno == EEXIST) {
         printf("EEXIST\n");
      } else if (errno == EINVAL) {
         printf("EINVAL\n");
      } else if (errno == ENOENT) {
         printf("ENOENT\n");
      } else if (errno == ENOMEM) {
         printf("ENOMEM\n");
      } else if (errno == ENOSPC) {
         printf("ENOSPC\n");
      } else if (errno == EPERM) {
         printf("EPERM\n");
      } else {
         printf("unrecognized error for EPOLL_CTL_DEL\n");
      }
      return false;
   } else {
      return true;
   }
#endif
   
   return false;
}

//******************************************************************************

bool EpollServer::isEventDisconnect(int eventIndex) {
#ifdef EPOLL_SUPPORT
   struct epoll_event current_event;
   current_event = m_events[eventIndex];
   return current_event.events & EPOLLHUP;
#endif
   
   return false;
}

//******************************************************************************

bool EpollServer::isEventReadClose(int eventIndex) {
#ifdef EPOLL_SUPPORT
   struct epoll_event current_event;
   current_event = m_events[eventIndex];
   return current_event.events & EPOLLRDHUP;
#endif
   
   return false;
}

//******************************************************************************

bool EpollServer::isEventRead(int eventIndex) {
#ifdef EPOLL_SUPPORT
   struct epoll_event current_event;
   current_event = m_events[eventIndex];
   return current_event.events & EPOLLIN;
#endif
   
   return false;
}

//******************************************************************************

