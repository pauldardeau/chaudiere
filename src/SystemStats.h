// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_SYSTEMSTATS_H
#define CHAUDIERE_SYSTEMSTATS_H

#include <string>

namespace chaudiere
{

/**
 * SystemStats retrieves various stats from the underlying OS
 */
class SystemStats
{
public:
   /**
    * Default constructor
    */
   SystemStats() noexcept;
   
   /**
    * Destructor
    */
   ~SystemStats() noexcept;

   /**
    * Retrieves the time that the system has been running in seconds
    * @param uptimeSeconds the variable to be populated
    * @return boolean indicating whether the uptime could be retrieved
    */
   static bool uptimeSeconds(long long& uptimeSeconds) noexcept;
   
   /**
    * Retrieves the system load averages for the past 1, 5, and 15 minute intervals
    * @param oneMinute the variable to populate with 1 minute load average
    * @param fiveMinute the variable to populate with 5 minute load average
    * @param fifteenMinute the variable to populate with 15 minute load average
    * @return boolean indicating whether the system load averages could be retrieved
    */
   static bool getLoadAverages(double& oneMinute,
                               double& fiveMinute,
                               double& fifteenMinute) noexcept;
                               
   /**
    * Retrieves the number of processes running on the system
    * @param numberProcesses the variable to populate
    * @return boolean indicating whether the number of processes could be retrieved
    */
   static bool getNumberProcesses(int& numberProcesses) noexcept;

   
   // disallow copies
   SystemStats(const SystemStats& copy) noexcept;
   SystemStats& operator=(const SystemStats& copy) noexcept;
   
   
};

}

#endif
