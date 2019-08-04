// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_STDLOGGER_H
#define CHAUDIERE_STDLOGGER_H

#include <string>
#include <unordered_map>
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
   
   /**
    *
    */
   LifecycleStats() :
      m_instancesCreated(0L),
      m_instancesDestroyed(0L) {
   }
   
   /**
    *
    * @param copy
    */
   LifecycleStats(const LifecycleStats& copy) :
      m_instancesCreated(copy.m_instancesCreated),
      m_instancesDestroyed(copy.m_instancesDestroyed) {
   }
   
   /**
    *
    * @param copy
    * @return
    */
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
   /**
    *
    */
   StdLogger();
   
   /**
    *
    * @param logLevel
    */
   StdLogger(LogLevel logLevel);
   
   /**
    *
    */
   virtual ~StdLogger();
   
   /**
    *
    * @return
    */
   virtual LogLevel getLogLevel() const;
   
   /**
    *
    * @param logLevel
    */
   virtual void setLogLevel(LogLevel logLevel);

   /**
    *
    * @param logLevel
    * @param logMessage
    */
   virtual void logMessage(LogLevel logLevel, const std::string& logMessage);
   
   /**
    *
    * @param logLevel
    * @return
    */
   virtual bool isLoggingLevel(LogLevel logLevel) const;
   
   /**
    *
    * @return
    */
   virtual bool isLoggingInstanceLifecycles() const;
   
   /**
    *
    * @param logInstanceLifecycles
    */
   virtual void setLogInstanceLifecycles(bool logInstanceLifecycles);
   
   /**
    *
    * @param className
    */
   virtual void logInstanceCreate(const std::string& className);
   
   /**
    *
    * @param className
    */
   virtual void logInstanceDestroy(const std::string& className);
   
   /**
    *
    * @param occurrenceType
    * @param occurrenceName
    */
   virtual void logOccurrence(const std::string& occurrenceType,
                              const std::string& occurrenceName);

   /**
    *
    * @param mapClassLifecycleStats
    */                           
   void populateClassLifecycleStats(std::unordered_map<std::string, LifecycleStats>& mapClassLifecycleStats);
   
   /**
    *
    * @param mapOccurrences
    */
   void populateOccurrences(std::unordered_map<std::string, std::unordered_map<std::string, long long> >& mapOccurrences);
   
   /**
    *
    * @param level
    * @return
    */
   const std::string& logLevelPrefix(LogLevel level) const;


private:
   std::unordered_map<std::string, LifecycleStats> m_mapClassLifecycleStats;
   std::unordered_map<std::string, std::unordered_map<std::string, long long> > m_mapOccurrences;
   std::unique_ptr<Mutex> m_lockLifecycleStats;
   std::unique_ptr<Mutex> m_lockOccurrences;
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
