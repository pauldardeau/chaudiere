// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "Logger.h"

using namespace chaudiere;

std::unique_ptr<Logger> Logger::loggerInstance = std::unique_ptr<Logger>(nullptr);

//******************************************************************************

void Logger::shutdown() {
   if (nullptr != loggerInstance) {
      loggerInstance.reset();
      loggerInstance = nullptr;
   }
}

//******************************************************************************

void Logger::setLogger(Logger* logger) {
   if (loggerInstance != nullptr) {
      loggerInstance.reset(logger);
   } else {
      loggerInstance = std::unique_ptr<Logger>(logger);
   }
}

//******************************************************************************

Logger* Logger::getLogger() {
   if (nullptr != loggerInstance) {
      return loggerInstance.get();
   } else {
      return nullptr;
   }
}

//******************************************************************************

void Logger::log(LogLevel logLevel, const std::string& logMessage) {
   if (loggerInstance) {
      loggerInstance->logMessage(logLevel, logMessage);
   }
}

//******************************************************************************

void Logger::critical(const std::string& logMessage) {
   log(Critical, logMessage);
}

//******************************************************************************

void Logger::error(const std::string& logMessage) {
   log(Error, logMessage);
}

//******************************************************************************

void Logger::warning(const std::string& logMessage) {
   log(Warning, logMessage);
}

//******************************************************************************

void Logger::info(const std::string& logMessage) {
   log(Info, logMessage);
}

//******************************************************************************

void Logger::debug(const std::string& logMessage) {
   log(Debug, logMessage);
}

//******************************************************************************

void Logger::verbose(const std::string& logMessage) {
   log(Verbose, logMessage);
}

//******************************************************************************

bool Logger::isLogging(LogLevel logLevel) {
   if (loggerInstance) {
      return loggerInstance->isLoggingLevel(logLevel);
   }
   
   return false;
}

//******************************************************************************

void Logger::logInstanceCreate(const char* className) {
   if (loggerInstance) {
      if (loggerInstance->isLoggingInstanceLifecycles()) {
         loggerInstance->logInstanceCreate(std::string(className));
      }
   }
}

//******************************************************************************

void Logger::logInstanceDestroy(const char* className) {
   if (loggerInstance) {
      if (loggerInstance->isLoggingInstanceLifecycles()) {
         loggerInstance->logInstanceDestroy(std::string(className));
      }
   }
}

//******************************************************************************

void Logger::countOccurrence(const char* occurrenceType,
                             const char* occurrenceName) {
   if (loggerInstance) {
      loggerInstance->logOccurrence(std::string(occurrenceType),
                                    std::string(occurrenceName));
   }
}

//******************************************************************************

void Logger::countOccurrence(const std::string& occurrenceType,
                             const std::string& occurrenceName) {
   if (loggerInstance) {
      loggerInstance->logOccurrence(occurrenceType, occurrenceName);
   }
}

//******************************************************************************

