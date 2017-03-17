// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <string.h>

#include "ServiceInfo.h"

using namespace chaudiere;

//******************************************************************************

ServiceInfo::ServiceInfo() :
   m_port(0),
   m_persistentConnection(false) {
}

//******************************************************************************

ServiceInfo::ServiceInfo(const std::string& serviceName,
                         const std::string& host,
                         unsigned short port) :
   m_serviceName(serviceName),
   m_host(host),
   m_port(port),
   m_persistentConnection(false) {
}

//******************************************************************************

ServiceInfo::ServiceInfo(const ServiceInfo& copy) :
   m_serviceName(copy.m_serviceName),
   m_host(copy.m_host),
   m_port(copy.m_port),
   m_persistentConnection(copy.m_persistentConnection) {
}

//******************************************************************************

ServiceInfo::~ServiceInfo() {
}

//******************************************************************************
   
ServiceInfo& ServiceInfo::operator=(const ServiceInfo& copy) {
   if (this == &copy) {
      return *this;
   }
   
   m_serviceName = copy.m_serviceName;
   m_host = copy.m_host;
   m_port = copy.m_port;
   m_persistentConnection = copy.m_persistentConnection;
   
   return *this;
}

//******************************************************************************

const std::string& ServiceInfo::serviceName() const {
   return m_serviceName;
}

//******************************************************************************

const std::string& ServiceInfo::host() const {
   return m_host;
}

//******************************************************************************

unsigned short ServiceInfo::port() const {
   return m_port;
}
   
//******************************************************************************

void ServiceInfo::setServiceName(const std::string& serviceName) {
   m_serviceName = serviceName;
}

//******************************************************************************

void ServiceInfo::setHost(const std::string& host) {
   m_host = host;
}

//******************************************************************************

void ServiceInfo::setPort(unsigned short port) {
   m_port = port;
}

//******************************************************************************

std::string ServiceInfo::getUniqueIdentifier() const {
   std::string uniqueIdentifier = m_host;
   uniqueIdentifier += std::string("|");
   
   char portAsString[10];
   memset(portAsString, 0, 10);
   snprintf(portAsString, 10, "%d", m_port);
   
   uniqueIdentifier += std::string(portAsString);
   
   return uniqueIdentifier;
}

//******************************************************************************

void ServiceInfo::setPersistentConnection(bool persistentConnection) {
   m_persistentConnection = persistentConnection;
}

//******************************************************************************

bool ServiceInfo::getPersistentConnection() const {
   return m_persistentConnection;
}

//******************************************************************************
