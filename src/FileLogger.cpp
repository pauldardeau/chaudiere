// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License
// FileLogger.cpp

#include "FileLogger.h"

using namespace chaudiere;

const std::string FileLogger::prefixCritical = "Critical:";
const std::string FileLogger::prefixError    = "Error:";
const std::string FileLogger::prefixWarning  = "Warning:";
const std::string FileLogger::prefixInfo     = "Info:";
const std::string FileLogger::prefixDebug    = "Debug:";
const std::string FileLogger::prefixVerbose  = "Verbose:";


//******************************************************************************

FileLogger::FileLogger(const std::string& filePath) noexcept :
   FileLogger(filePath, Logger::LogLevel::Debug)
{
}

//******************************************************************************

FileLogger::FileLogger(const std::string& filePath, LogLevel logLevel) noexcept :
   m_filePath(filePath),
   f(nullptr),
   m_logLevel(logLevel)
{
}

//******************************************************************************

FileLogger::~FileLogger() noexcept
{
   if (f != nullptr) {
      ::fclose(f);
   }
}

//******************************************************************************

Logger::LogLevel FileLogger::getLogLevel() const noexcept
{
   return m_logLevel;
}

//******************************************************************************

void FileLogger::setLogLevel(LogLevel logLevel) noexcept
{
   m_logLevel = logLevel;
}

//******************************************************************************

void FileLogger::logMessage(LogLevel logLevel, const std::string& logMessage) noexcept
{
   if (isLogging(logLevel)) {
      if (f == nullptr) {
         f = fopen(m_filePath.c_str(), "a+");
      }
      
      if (f != nullptr) {
         fprintf(f, "%s %s\n", logLevelPrefix(logLevel).c_str(), logMessage.c_str());
      }
   }
}

//******************************************************************************

bool FileLogger::isLoggingLevel(LogLevel logLevel) const noexcept
{
   return (logLevel <= m_logLevel);
}

//******************************************************************************

const std::string& FileLogger::logLevelPrefix(LogLevel level) const noexcept
{
   switch (level) {
      case Logger::LogLevel::Critical:
         return prefixCritical;
      case Logger::LogLevel::Error:
         return prefixError;
      case Logger::LogLevel::Warning:
         return prefixWarning;
      case Logger::LogLevel::Info:
         return prefixInfo;
      case Logger::LogLevel::Verbose:
         return prefixVerbose;
      case Logger::LogLevel::Debug:
      default:
         return prefixDebug;
   }
}

//******************************************************************************

bool FileLogger::isLoggingInstanceLifecycles() const noexcept
{
   return false;
}

//******************************************************************************

void FileLogger::setLogInstanceLifecycles(bool logInstanceLifecycles) noexcept
{
}

//******************************************************************************

void FileLogger::logInstanceCreate(const std::string& className) noexcept
{
}

//******************************************************************************

void FileLogger::logInstanceDestroy(const std::string& className) noexcept
{
}

//******************************************************************************

void FileLogger::logOccurrence(const std::string& occurrenceType,
                               const std::string& occurrenceName) noexcept
{
}

//******************************************************************************

