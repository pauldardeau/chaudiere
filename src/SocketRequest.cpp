// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>

#include "SocketRequest.h"
#include "SocketServiceHandler.h"
#include "Logger.h"
#include "BasicException.h"

using namespace chaudiere;

//******************************************************************************

SocketRequest::SocketRequest(Socket* socket,
                             SocketServiceHandler* handler) :
   Runnable(),
   m_socket(socket),
   m_borrowedSocket(socket),
   m_handler(handler),
   m_containedSocket(-1),  // not used
   m_socketOwned(true) {
   LOG_INSTANCE_CREATE("SocketRequest")
}

//******************************************************************************

SocketRequest::SocketRequest(SocketCompletionObserver* completionObserver,
                             int socketFD,
                             SocketServiceHandler* handler) :
   Runnable(),
   m_socket(NULL),
   m_borrowedSocket(NULL),  // not used
   m_handler(handler),
   m_containedSocket(completionObserver, socketFD),
   m_socketOwned(false) {
   LOG_INSTANCE_CREATE("SocketRequest")
   m_socket = &m_containedSocket;
}

//******************************************************************************

SocketRequest::~SocketRequest() {
   LOG_INSTANCE_DESTROY("SocketRequest")
   if (NULL != m_borrowedSocket) {
      delete m_borrowedSocket;
      m_borrowedSocket = NULL;
      m_socket = NULL;
   }
}

//******************************************************************************

void SocketRequest::run() {
   if (Logger::isLogging(Debug)) {
      char msg[128];
      ::snprintf(msg, 128, "request for socket fd=%d",
               m_socket->getFileDescriptor());
      LOG_DEBUG(msg)
   }

   if (m_handler) {
      try {
         SocketRequest* socketRequest =
            dynamic_cast<SocketRequest*>(this);
         m_handler->serviceSocket(socketRequest);
      } catch (const BasicException& be) {
         LOG_ERROR("exception in serviceSocket on handler: " + be.whatString())
      } catch (const std::exception& e) {
         LOG_ERROR("exception in serviceSocket on handler: " + std::string(e.what()))
      } catch (...) {
         LOG_ERROR("exception in serviceSocket on handler")
      }
   } else {
      LOG_ERROR("no handler present in SocketRequest")
   }
}

//******************************************************************************

int SocketRequest::getSocketFD() const {
   return m_socket->getFileDescriptor();
}

//******************************************************************************

Socket* SocketRequest::getSocket() {
   return m_socket;
}

//******************************************************************************

void SocketRequest::requestComplete() {
   if (NULL != m_socket) {
      m_socket->requestComplete();
   }
}

//******************************************************************************

bool SocketRequest::isSocketOwned() const {
   return m_socketOwned;
}

//******************************************************************************

void SocketRequest::setSocketOwned(bool socketOwned) {
   m_socketOwned = socketOwned;
}

//******************************************************************************

void SocketRequest::setUserIndex(int userIndex) {
   m_socket->setUserIndex(userIndex);
}

//******************************************************************************

void SocketRequest::notifyOnCompletion() {
   requestComplete();
   Runnable::notifyOnCompletion();
   //requestComplete();
}

//******************************************************************************

