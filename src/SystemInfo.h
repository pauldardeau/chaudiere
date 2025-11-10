// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_SYSTEMINFO_H
#define CHAUDIERE_SYSTEMINFO_H

#include <string>

namespace chaudiere
{

/**
 * Wrapper/utility class around uname function
 */
class SystemInfo
{
public:
   /**
    * Default constructor and retrieves the system information
    */
   SystemInfo();

   /**
    * Copy constructor
    * @param copy the source of the copy
    */
   SystemInfo(const SystemInfo& copy);

   /**
    * Destructor
    */
   ~SystemInfo();

   /**
    * Copy operator
    * @param copy the source of the copy
    * @return reference to the updated instance
    */
   SystemInfo& operator=(const SystemInfo& copy);

   /**
    * Retrieves the system name (sysname)
    * @return the system name
    */
   const std::string& sysName() const;

   /**
    * Retrieves the node name (nodename)
    * @return the node name
    */
   const std::string& nodeName() const;

   /**
    * Retrieves the release (release)
    * @return the release
    */
   const std::string& release() const;

   /**
    * Retrieves the version (version)
    * @return the version
    */
   const std::string& version() const;

   /**
    * Retrieves the machine (machine)
    * @return the machine
    */
   const std::string& machine() const;

   /**
    * Determines if the system information was able to be retrieved
    * @return boolean indicating whether the system information was successfully read
    */
   bool retrievedSystemInfo() const;


private:
   std::string m_sysName;
   std::string m_nodeName;
   std::string m_release;
   std::string m_version;
   std::string m_machine;
   bool m_retrievedSystemInfo;

};

}

#endif
