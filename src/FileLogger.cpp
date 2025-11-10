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

FileLogger::FileLogger(const std::string& filePath) :
   m_filePath(filePath),
   f(nullptr),
   m_logLevel(Debug) {
}

//******************************************************************************

FileLogger::FileLogger(const std::string& filePath, LogLevel logLevel) :
   m_filePath(filePath),
   f(nullptr),
   m_logLevel(logLevel) {
}

//******************************************************************************

FileLogger::~FileLogger() {
   if (f != nullptr) {
      ::fclose(f);
   }
}

//******************************************************************************

LogLevel FileLogger::getLogLevel() const {
   return m_logLevel;
}

//******************************************************************************

void FileLogger::setLogLevel(LogLevel logLevel) {
   m_logLevel = logLevel;
}

//******************************************************************************

void FileLogger::logMessage(LogLevel logLevel,
                            const std::string& logMessage) {
   if (isLogging(logLevel)) {
      if (f == nullptr) {
         f = ::fopen(m_filePath.c_str(), "a+");
      }

      if (f != nullptr) {
         ::fprintf(f, "%s %s\n",
                      logLevelPrefix(logLevel).c_str(),
                      logMessage.c_str());
      }
   }
}

//******************************************************************************

bool FileLogger::isLoggingLevel(LogLevel logLevel) const {
   return (logLevel <= m_logLevel);
}

//******************************************************************************

const std::string& FileLogger::logLevelPrefix(LogLevel level) const {
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

bool FileLogger::isLoggingInstanceLifecycles() const {
   return false;
}

//******************************************************************************

void FileLogger::setLogInstanceLifecycles(bool logInstanceLifecycles) {
}

//******************************************************************************

void FileLogger::logInstanceCreate(const std::string& className) {
}

//******************************************************************************

void FileLogger::logInstanceDestroy(const std::string& className) {
}

//******************************************************************************

void FileLogger::logOccurrence(const std::string& occurrenceType,
                               const std::string& occurrenceName) {
}

//******************************************************************************

