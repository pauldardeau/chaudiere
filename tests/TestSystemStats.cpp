// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "TestSystemStats.h"
#include "SystemStats.h"

using namespace chaudiere;

//******************************************************************************

TestSystemStats::TestSystemStats() :
   poivre::TestSuite("TestSystemStats") {
}

//******************************************************************************

void TestSystemStats::runTests() {
   testUptimeSeconds();
   testGetLoadAverages();
   testGetNumberProcesses();
}

//******************************************************************************

void TestSystemStats::testUptimeSeconds() {
   TEST_CASE("testUptimeSeconds");

   long long uptime = -1;
   require(SystemStats::uptimeSeconds(uptime), "uptimeSeconds should succeed on a real Linux/Unix host");
   require(uptime >= 0, "uptime should be a non-negative number of seconds");
}

//******************************************************************************

void TestSystemStats::testGetLoadAverages() {
   TEST_CASE("testGetLoadAverages");

   double oneMinute = -1.0;
   double fiveMinute = -1.0;
   double fifteenMinute = -1.0;
   require(SystemStats::getLoadAverages(oneMinute, fiveMinute, fifteenMinute), "getLoadAverages should succeed on a real Linux/Unix host");
   require(oneMinute >= 0.0, "1-minute load average should be non-negative");
   require(fiveMinute >= 0.0, "5-minute load average should be non-negative");
   require(fifteenMinute >= 0.0, "15-minute load average should be non-negative");
}

//******************************************************************************

void TestSystemStats::testGetNumberProcesses() {
   TEST_CASE("testGetNumberProcesses");

#ifdef __linux__
   // SystemStats.cpp has no Linux implementation of getNumberProcesses()
   // (its body is entirely #ifndef __linux__, guarding a BSD/macOS-only
   // sysctl(KERN_PROC_ALL) call), so it always returns false here.
   int numberProcesses = 0;
   requireFalse(SystemStats::getNumberProcesses(numberProcesses), "getNumberProcesses has no Linux implementation and should report failure");
#else
   int numberProcesses = 0;
   require(SystemStats::getNumberProcesses(numberProcesses), "getNumberProcesses should succeed on a real BSD/macOS host");
   require(numberProcesses > 0, "there should be at least this process running");
#endif
}

//******************************************************************************
