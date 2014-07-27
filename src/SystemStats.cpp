// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdlib.h>

#include "SystemStats.h"

using namespace chaudiere;

//******************************************************************************

bool SystemStats::uptimeSeconds(long long& uptimeSeconds) noexcept
{
   //TODO: implement uptimeSeconds
   return false;
}

//******************************************************************************

bool SystemStats::getLoadAverages(double& oneMinute,
                                  double& fiveMinute,
                                  double& fifteenMinute) noexcept
{
   bool retrievedLoadAverages = false;
   
   double load[3];
   const int numSamples = ::getloadavg(load, 3);
   
   if (numSamples == 3) {
      oneMinute = load[0];
      fiveMinute = load[1];
      fifteenMinute = load[2];
      retrievedLoadAverages = true;
   }
   
   return retrievedLoadAverages;
}

//******************************************************************************

bool SystemStats::getNumberProcesses(int& numberProcesses) noexcept
{
   //TODO: implement getNumberProcesses
   return false;
}

//******************************************************************************

