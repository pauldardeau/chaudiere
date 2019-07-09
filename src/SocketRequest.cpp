// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>

#include "SocketRequest.h"
#include "Socket.h"
#include "SocketServiceHandler.h"
#include "Logger.h"
#include "BasicException.h"

using namespace chaudiere;

//******************************************************************************

SocketRequest::SocketRequest(Socket* socket,
                             SocketServiceHandler* handler) :
   Runnable(),
   m_socket(socket),
   m_handler(handler),
   m_socketOwned(true) {
   Logger::logInstanceCreate("SocketRequest");
}

//******************************************************************************

SocketRequest::~SocketRequest() {
   Logger::logInstanceDestroy("SocketRequest");
}

//******************************************************************************

void SocketRequest::run() {
   if (Logger::isLogging(Debug)) {
      char msg[128];
      ::snprintf(msg, 128, "request for socket fd=%d",
               m_socket->getFileDescriptor());
      Logger::debug(std::string(msg));
   }

   if (m_handler) {
      try {
         SocketRequest* socketRequest =
            dynamic_cast<SocketRequest*>(this);
         m_handler->serviceSocket(socketRequest);
      } catch (const BasicException& be) {
         Logger::error("exception in serviceSocket on handler: " + be.whatString());
      } catch (const std::exception& e) {
         Logger::error("exception in serviceSocket on handler: " + std::string(e.what()));
      } catch (...) {
         Logger::error("exception in serviceSocket on handler");
      }
   } else {
      Logger::error("no handler present in SocketRequest");
   }
    
   //m_socket->requestComplete();
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

void SocketRequest::setSocketOwned(bool socketOwned) {
   m_socketOwned = socketOwned;
}

