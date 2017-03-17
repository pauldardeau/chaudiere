// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "RequestHandler.h"
#include "Socket.h"
#include "SocketRequest.h"
#include "Thread.h"
#include "BasicException.h"
#include "Logger.h"
#include "StrUtils.h"

using namespace chaudiere;

//******************************************************************************

RequestHandler::RequestHandler(SocketRequest* socketRequest) :
   m_socket(NULL),
   m_socketRequest(socketRequest),
   m_isThreadPooling(false) {
   Logger::logInstanceCreate("RequestHandler");
}

//******************************************************************************

RequestHandler::RequestHandler(Socket* socket) :
   m_socket(socket),
   m_socketRequest(NULL),
   m_isThreadPooling(false) {
   Logger::logInstanceCreate("RequestHandler");
}

//******************************************************************************

RequestHandler::~RequestHandler() {
   Logger::logInstanceDestroy("RequestHandler");

   if (m_socket) {
      m_socket->close();
   }
}

//******************************************************************************

void RequestHandler::setThreadPooling(bool isThreadPooling) {
   m_isThreadPooling = isThreadPooling;
}

//******************************************************************************

bool RequestHandler::isThreadPooling() const {
   return m_isThreadPooling;
}

//******************************************************************************

Socket* RequestHandler::getSocket() {
   Socket* socket = NULL;

   if (m_socket) {
      socket = m_socket;
   } else if (m_socketRequest) {
      socket = m_socketRequest->getSocket();
   }
   
   return socket;
}

//******************************************************************************
