// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef __HttpServer__SystemStats__
#define __HttpServer__SystemStats__

#include <string>


/*!
 *
 */
class SystemStats
{
public:
   SystemStats() noexcept;
   ~SystemStats() noexcept;

   
   static bool uptimeSeconds(long long& uptimeSeconds) noexcept;
   static bool getLoadAverages(double& oneMinute,
                               double& fiveMinute,
                               double& fifteenMinute) noexcept;
   static bool getNumberProcesses(int& numberProcesses) noexcept;

   
   // disallow copies
   SystemStats(const SystemStats& copy) noexcept;
   SystemStats(SystemStats&& move) noexcept;
   SystemStats& operator=(const SystemStats& copy) noexcept;
   SystemStats& operator=(SystemStats&& move) noexcept;
   
   
};

#endif /* defined(__HttpServer__SystemStats__) */
