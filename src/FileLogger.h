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
   FileLogger(const std::string& filePath) noexcept;
   FileLogger(const std::string& filePath, LogLevel logLevel) noexcept;
   virtual ~FileLogger() noexcept;
   
   virtual LogLevel getLogLevel() const noexcept override;
   virtual void setLogLevel(LogLevel logLevel) noexcept override;

   virtual void logMessage(LogLevel logLevel,
                           const std::string& logMessage) noexcept override;
   virtual bool isLoggingLevel(LogLevel logLevel) const noexcept override;
   
   virtual bool isLoggingInstanceLifecycles() const noexcept override;
   virtual void setLogInstanceLifecycles(bool logInstanceLifecycles) noexcept override;
   virtual void logInstanceCreate(const std::string& className) noexcept override;
   virtual void logInstanceDestroy(const std::string& className) noexcept override;
   
   virtual void logOccurrence(const std::string& occurrenceType,
                              const std::string& occurrenceName) noexcept override;

   const std::string& logLevelPrefix(LogLevel level) const noexcept;


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
