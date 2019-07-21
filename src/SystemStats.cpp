// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>

#ifdef __FreeBSD__
#include <kvm.h>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/sysctl.h>
#endif

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef __OpenBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

#include "SystemStats.h"

using namespace chaudiere;

//******************************************************************************

bool SystemStats::uptimeSeconds(long long& uptimeSeconds) {
   bool success = false;
   
#ifdef __linux__
   struct sysinfo si;
   ::memset(&si, 0, sizeof(si));
   
   const int rc = ::sysinfo(&si);
   
   if (rc == 0) {
      uptimeSeconds = si.uptime;  // seconds since boot
      success = true;
   }
#else
#if defined(__FreeBSD__)
   struct timeval boottime;
   size_t len = sizeof(boottime);
   int mib[2] = { CTL_KERN, KERN_BOOTTIME };
   if (::sysctl(mib, 2, &boottime, &len, NULL, 0) < 0) {
      return false;
   }
   
   time_t bsec = boottime.tv_sec;
   time_t csec = ::time(NULL);

   uptimeSeconds = ::difftime(csec, bsec);
   
   success = true;
#endif
#endif
          
   return success;
}

//******************************************************************************

bool SystemStats::getLoadAverages(double& oneMinute,
                                  double& fiveMinute,
                                  double& fifteenMinute) {
   bool retrievedLoadAverages = false;
#ifndef __sun
   double load[3];
   const int numSamples = ::getloadavg(load, 3);
   
   if (numSamples == 3) {
      oneMinute = load[0];
      fiveMinute = load[1];
      fifteenMinute = load[2];
      retrievedLoadAverages = true;
   }
#endif
   return retrievedLoadAverages;
}

//******************************************************************************

bool SystemStats::getNumberProcesses(int& numberProcesses) {
   bool success = false;

#ifdef __NetBSD__
   //TODO: port for NetBSD
   return false;
#endif
   
#ifndef __linux__
#ifndef __NetBSD__
#ifndef __sun
   int rc;
   size_t length = 0;
   static const int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
   
   // Call sysctl with a NULL buffer to get proper length
   rc = ::sysctl((int *)name,
                 (sizeof(name) / sizeof(*name)) - 1,
                 NULL,
                 &length,
                 NULL,
                 0);
   
   if (rc == 0) {
      // Allocate buffer
      struct kinfo_proc* proc_list =
         (struct kinfo_proc*)::malloc(length);
       
      if (proc_list != NULL) {
         // Get the actual process list
         rc = ::sysctl((int *)name,
                       (sizeof(name) / sizeof(*name)) - 1,
                       proc_list,
                       &length,
                       NULL,
                       0);
         if (rc == 0) {
            numberProcesses = length / sizeof(struct kinfo_proc);
            success = true;
         }
         
         ::free(proc_list);
      }
   }
#endif
#endif
#endif
      
   return success;
}

//******************************************************************************

