// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_FILELOGGER_H
#define CHAUDIERE_FILELOGGER_H

#include <stdio.h>
#include <string>

#include "Logger.h"

namespace chaudiere
{

/**
 *
 */
class FileLogger : public Logger
{
public:
   FileLogger(const std::string& filePath);
   FileLogger(const std::string& filePath, LogLevel logLevel);
   virtual ~FileLogger();
   
   virtual LogLevel getLogLevel() const;
   virtual void setLogLevel(LogLevel logLevel);

   virtual void logMessage(LogLevel logLevel,
                           const std::string& logMessage);
   virtual bool isLoggingLevel(LogLevel logLevel) const;
   
   virtual bool isLoggingInstanceLifecycles() const;
   virtual void setLogInstanceLifecycles(bool logInstanceLifecycles);
   virtual void logInstanceCreate(const std::string& className);
   virtual void logInstanceDestroy(const std::string& className);
   
   virtual void logOccurrence(const std::string& occurrenceType,
                              const std::string& occurrenceName);

   const std::string& logLevelPrefix(LogLevel level) const;


private:
   std::string m_filePath;
   FILE* f;
   LogLevel m_logLevel;
   
   static const std::string prefixCritical;
   static const std::string prefixError;
   static const std::string prefixWarning;
   static const std::string prefixInfo;
   static const std::string prefixDebug;
   static const std::string prefixVerbose;
   
   // disallow copies
   FileLogger(const FileLogger&);
   FileLogger& operator=(const FileLogger&);

};

}

#endif
