// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_STDLOGGER_H
#define CHAUDIERE_STDLOGGER_H

#include <string>
#include <map>
#include <memory>

#include "Logger.h"
#include "Mutex.h"

namespace chaudiere
{

/**
 *
 */
class LifecycleStats
{
public:
   long long m_instancesCreated;
   long long m_instancesDestroyed;
   
   LifecycleStats() :
      m_instancesCreated(0L),
      m_instancesDestroyed(0L) {
   }
   
   LifecycleStats(const LifecycleStats& copy) :
      m_instancesCreated(copy.m_instancesCreated),
      m_instancesDestroyed(copy.m_instancesDestroyed) {
   }
   
   LifecycleStats& operator=(const LifecycleStats& copy) {
      if (this == &copy) {
         return *this;
      }
      
      m_instancesCreated = copy.m_instancesCreated;
      m_instancesDestroyed = copy.m_instancesDestroyed;
      
      return *this;
   }
};

/**
 * StdLogger is the default logger and logs to the console.
 */
class StdLogger : public Logger
{
public:
   StdLogger() noexcept;
   StdLogger(LogLevel logLevel) noexcept;
   virtual ~StdLogger() noexcept;
   
   virtual LogLevel getLogLevel() const noexcept override;
   virtual void setLogLevel(LogLevel logLevel) noexcept override;

   virtual void logMessage(LogLevel logLevel, const std::string& logMessage) noexcept override;
   virtual bool isLoggingLevel(LogLevel logLevel) const noexcept override;
   
   virtual bool isLoggingInstanceLifecycles() const noexcept override;
   virtual void setLogInstanceLifecycles(bool logInstanceLifecycles) noexcept override;
   virtual void logInstanceCreate(const std::string& className) noexcept override;
   virtual void logInstanceDestroy(const std::string& className) noexcept override;
   
   virtual void logOccurrence(const std::string& occurrenceType,
                              const std::string& occurrenceName) noexcept override;

   void populateClassLifecycleStats(std::map<std::string, LifecycleStats>& mapClassLifecycleStats);
   void populateOccurrences(std::map<std::string, std::map<std::string, long long>>& mapOccurrences);
   
   const std::string& logLevelPrefix(LogLevel level) const noexcept;


private:
   std::map<std::string, LifecycleStats> m_mapClassLifecycleStats;
   std::map<std::string, std::map<std::string, long long>> m_mapOccurrences;
   Mutex* m_lockLifecycleStats;
   Mutex* m_lockOccurrences;
   LogLevel m_logLevel;
   bool m_isLoggingInstanceLifecycles;
   
   static const std::string prefixCritical;
   static const std::string prefixError;
   static const std::string prefixWarning;
   static const std::string prefixInfo;
   static const std::string prefixDebug;
   static const std::string prefixVerbose;
   
   // disallow copies
   StdLogger(const StdLogger&);
   StdLogger& operator=(const StdLogger&);

};

}

#endif
