// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <string>

#include "KqueueServer.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

bool KqueueServer::isSupportedPlatform() {
#ifdef KQUEUE_SUPPORT
   return true;
#else
   return false;
#endif
}

//******************************************************************************

KqueueServer::KqueueServer(Mutex& fdMutex, Mutex& hwmConnectionsMutex) :
   KernelEventServer(fdMutex, hwmConnectionsMutex, "KqueueServer"),
   m_kqfd(-1) {
   Logger::logInstanceCreate("KqueueServer");
}

//******************************************************************************

KqueueServer::~KqueueServer() {
   Logger::logInstanceDestroy("KqueueServer");

   if (-1 != m_kqfd) {
      ::close(m_kqfd);
   }
}

//******************************************************************************

bool KqueueServer::init(SocketServiceHandler* socketServiceHandler,
                        int serverPort,
                        int maxConnections) {
#ifndef KQUEUE_SUPPORT
   return false;
#endif

#ifdef KQUEUE_SUPPORT
   if (KernelEventServer::init(socketServiceHandler, serverPort, maxConnections)) {
      m_kqfd = ::kqueue();
      if (m_kqfd == -1) {
         Logger::critical("kqueue create failed");
         return false;
      }
      
      // add our listener socket (server socket) as one of the fd's that
      // we want watched
      struct kevent ev;
      EV_SET(&ev,
             getListenerSocketFileDescriptor(),
             EVFILT_READ,
             EV_ADD,
             0, 0, NULL);
      
      if (::kevent(m_kqfd, &ev, 1, 0, 0, NULL) < 0) {
         Logger::critical("kevent failed for adding read filter");
         return false;
      } else {
         return true;
      }
   }
#endif
   
   return false;
}

//******************************************************************************

int KqueueServer::getKernelEvents(int maxConnections) {
#ifdef KQUEUE_SUPPORT
   //printf("waiting on kevent\n");
   const int numberEventsReturned =
      ::kevent(m_kqfd,
               NULL, 0,
               &m_event, 1,
               (struct timespec*) 0);
   if (-1 == numberEventsReturned) {
      Logger::critical("unable to retrieve events from kevent");
      return -1;
   }

   //printf("have kevent\n");
   return numberEventsReturned;
#else
   return 0;
#endif
}

//******************************************************************************

int KqueueServer::fileDescriptorForEventIndex(int eventIndex) {
   int client_fd = -1;
   
#ifdef KQUEUE_SUPPORT
   client_fd = (int) m_event.ident;
#endif
   
   return client_fd;
}

//******************************************************************************

bool KqueueServer::addFileDescriptorForRead(int fileDescriptor) {
#ifdef KQUEUE_SUPPORT
   //printf("adding fd=%d\n", fileDescriptor);
   struct kevent ev;
   EV_SET(&ev, fileDescriptor, EVFILT_READ, EV_ADD, 0, 0, NULL);
   
   if (::kevent(m_kqfd, &ev, 1, NULL, 0, NULL) < 0) {
      Logger::critical("kevent failed adding read filter");
   } else {
      return true;
   }
#endif
   
   return false;
}

//******************************************************************************

bool KqueueServer::removeFileDescriptorFromRead(int fileDescriptor) {
#ifdef KQUEUE_SUPPORT
   struct kevent ev;
   //printf("removing fd=%d\n", fileDescriptor);
   EV_SET(&ev, fileDescriptor, EVFILT_READ, EV_DELETE, 0, 0, NULL);
   
   if (::kevent(m_kqfd, &ev, 1, NULL, 0, NULL) < 0) {
      Logger::warning("kevent failed to delete read filter");
      return false;
   } else {
      return true;
   }
#endif

   return false;
}

//******************************************************************************

bool KqueueServer::isEventDisconnect(int eventIndex) {
#ifdef KQUEUE_SUPPORT
   return m_event.flags & EV_EOF;
#endif

   return false;
}

//******************************************************************************

bool KqueueServer::isEventReadClose(int eventIndex) {
#ifdef KQUEUE_SUPPORT
   return m_event.flags & EV_EOF;
#endif

   return false;
}

//******************************************************************************

bool KqueueServer::isEventRead(int eventIndex) {
#ifdef KQUEUE_SUPPORT
   return m_event.filter == EVFILT_READ;
#endif

   return false;
}

//******************************************************************************

