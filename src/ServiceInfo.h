// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef SERVICEINFO_H
#define SERVICEINFO_H

#include <string>


/**
 * ServiceInfo captures basic information for a network service such as the service
 * name, host that provides the service, and the port number.
 */
class ServiceInfo
{
public:
   /**
    * Default constructor
    */
   ServiceInfo();
   
   /**
    * Initialize ServiceInfo instance with service name, host, and port number
    * @param serviceName the name of the service
    * @param host the host for the service
    * @param port the port number for the service
    */
   ServiceInfo(const std::string& serviceName, const std::string& host, unsigned short port);
   
   /**
    * Copy constructor
    * @param copy the source of the copy
    */
   ServiceInfo(const ServiceInfo& copy);
   
   /**
    * Move constructor
    * @param move the source of the move
    */
   ServiceInfo(ServiceInfo&& move);
   
   /**
    * Destructor
    */
   ~ServiceInfo();
   
   /**
    * Copy operator
    * @param copy the source of the copy
    * @return reference to the target of the copy
    */
   ServiceInfo& operator=(const ServiceInfo& copy);
   
   /**
    * Move operator
    * @param move the source of the move
    * @return reference to the target of the move
    */
   ServiceInfo& operator=(ServiceInfo&& move);
   
   /**
    * Retrieves the name of the sevice
    * @return the service name
    */
   const std::string& serviceName() const;
   
   /**
    * Retrieves the host for the service
    * @return the service host
    */
   const std::string& host() const;
   
   /**
    * Retrieves the port number for the service
    * @return the service port number
    */
   unsigned short port() const;
   
   /**
    * Updates the name of the service
    * @param serviceName new name for the service
    */
   void setServiceName(const std::string& serviceName);
   
   /**
    * Updates the host for the service
    * @param host the new host for the service
    */
   void setHost(const std::string& host);
   
   /**
    * Updates the port number for the service
    * @param port the new port number for the service
    */
   void setPort(unsigned short port);


private:
   std::string m_serviceName;
   std::string m_host;
   unsigned short m_port;
};

#endif
