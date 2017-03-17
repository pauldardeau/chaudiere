// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include "StdLogger.h"
#include "PthreadsMutex.h"
#include "MutexLock.h"

using namespace chaudiere;

const std::string StdLogger::prefixCritical = "Critical:";
const std::string StdLogger::prefixError    = "Error:";
const std::string StdLogger::prefixWarning  = "Warning:";
const std::string StdLogger::prefixInfo     = "Info:";
const std::string StdLogger::prefixDebug    = "Debug:";
const std::string StdLogger::prefixVerbose  = "Verbose:";


//******************************************************************************

StdLogger::StdLogger() :
   m_lockLifecycleStats(new PthreadsMutex("lockLifecycleStats")),
   m_lockOccurrences(new PthreadsMutex("lockOccurrences")),
   m_logLevel(Debug),
   m_isLoggingInstanceLifecycles(false) {
}

//******************************************************************************

StdLogger::StdLogger(LogLevel logLevel) :
   m_lockLifecycleStats(new PthreadsMutex("lockLifecycleStats")),
   m_lockOccurrences(new PthreadsMutex("lockOccurrences")),
   m_logLevel(logLevel),
   m_isLoggingInstanceLifecycles(false) {
}

//******************************************************************************

StdLogger::~StdLogger() {
   delete m_lockLifecycleStats;
   delete m_lockOccurrences;
}

//******************************************************************************

LogLevel StdLogger::getLogLevel() const {
   return m_logLevel;
}

//******************************************************************************

void StdLogger::setLogLevel(LogLevel logLevel) {
   m_logLevel = logLevel;
}

//******************************************************************************

void StdLogger::logMessage(LogLevel logLevel,
                           const std::string& logMessage) {
   if (isLogging(logLevel)) {
      std::printf("%s %s\n",
                  logLevelPrefix(logLevel).c_str(),
                  logMessage.c_str());
   }
}

//******************************************************************************

bool StdLogger::isLoggingLevel(LogLevel logLevel) const {
   return (logLevel <= m_logLevel);
}

//******************************************************************************

const std::string& StdLogger::logLevelPrefix(LogLevel level) const {
   switch (level) {
      case Critical:
         return prefixCritical;
      case Error:
         return prefixError;
      case Warning:
         return prefixWarning;
      case Info:
         return prefixInfo;
      case Verbose:
         return prefixVerbose;
      case Debug:
      default:
         return prefixDebug;
   }
}

//******************************************************************************

bool StdLogger::isLoggingInstanceLifecycles() const {
   return m_isLoggingInstanceLifecycles;
}

//******************************************************************************

void StdLogger::setLogInstanceLifecycles(bool logInstanceLifecycles) {
   m_isLoggingInstanceLifecycles = logInstanceLifecycles;
}

//******************************************************************************

void StdLogger::logInstanceCreate(const std::string& className) {
   if (m_lockLifecycleStats) {
      MutexLock lock(*m_lockLifecycleStats);
      std::map<std::string, LifecycleStats>::iterator it =
         m_mapClassLifecycleStats.find(className);
      if (it != m_mapClassLifecycleStats.end()) {
         LifecycleStats& stats = it->second;
         ++stats.m_instancesCreated;
      } else {
         LifecycleStats stats;
         ++stats.m_instancesCreated;
         m_mapClassLifecycleStats[className] = stats;
      }
   }
}

//******************************************************************************

void StdLogger::logInstanceDestroy(const std::string& className) {
   if (m_lockLifecycleStats) {
      MutexLock lock(*m_lockLifecycleStats);
      std::map<std::string, LifecycleStats>::iterator it =
         m_mapClassLifecycleStats.find(className);
      if (it != m_mapClassLifecycleStats.end()) {
         LifecycleStats& stats = it->second;
         ++stats.m_instancesDestroyed;
      } else {
         error("unable to find class lifecycle stats for destroy of " + className);
      }
   }
}

//******************************************************************************

void StdLogger::populateClassLifecycleStats(std::map<std::string,
                                                     LifecycleStats>& mapClassLifecycleStats) {
   MutexLock lock(*m_lockLifecycleStats);
   mapClassLifecycleStats = m_mapClassLifecycleStats;
}

//******************************************************************************

void StdLogger::populateOccurrences(std::map<std::string,
                                             std::map<std::string,
                                                      long long> >& mapOccurrences) {
   MutexLock lock(*m_lockOccurrences);
   mapOccurrences = m_mapOccurrences;
}

//******************************************************************************

void StdLogger::logOccurrence(const std::string& occurrenceType,
                              const std::string& occurrenceName) {
   MutexLock lock(*m_lockOccurrences);
   std::map<std::string, std::map<std::string, long long> >::iterator it =
      m_mapOccurrences.find(occurrenceType);
   if (it != m_mapOccurrences.end()) {
      std::map<std::string, long long>& mapOccurrencesByType = it->second;
      std::map<std::string, long long>::iterator itName =
         mapOccurrencesByType.find(occurrenceName);
      if (itName != mapOccurrencesByType.end()) {
         ++(itName->second);
      } else {
         mapOccurrencesByType[occurrenceName] = 1L;
      }
   } else {
      std::map<std::string, long long> mapOccurences;
      mapOccurences[occurrenceName] = 1L;
      m_mapOccurrences[occurrenceType] = mapOccurences;
   }
}

//******************************************************************************

