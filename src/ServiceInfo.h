// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_SERVICEINFO_H
#define CHAUDIERE_SERVICEINFO_H

#include <string>

namespace chaudiere
{

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
   ServiceInfo(const std::string& serviceName,
               const std::string& host,
               unsigned short port);
   
   /**
    * Copy constructor
    * @param copy the source of the copy
    */
   ServiceInfo(const ServiceInfo& copy);
   
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
    * Retrieves the name of the service
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
   
   /**
    * Retrieves a uniquely identifying string for the ServiceInfo instance
    * @return uniquely identifying string
    */
   std::string getUniqueIdentifier() const;
   
   /**
    * Sets boolean indicator as to whether connection should be persistent
    * @param persistentConnection boolean indicating whether connection should be persistent
    */
   void setPersistentConnection(bool persistentConnection);
   
   /**
    * Retrieves boolean value indicating whether service connection should be persistent
    * @return boolean value indicating whether connection should be persistent
    */
   bool getPersistentConnection() const;


private:
   std::string m_serviceName;
   std::string m_host;
   unsigned short m_port;
   bool m_persistentConnection;
};

}

#endif
