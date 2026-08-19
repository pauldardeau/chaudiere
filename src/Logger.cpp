// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <memory>

#include "Logger.h"

using namespace chaudiere;

std::shared_ptr<Logger> Logger::loggerInstance;

//******************************************************************************

// loggerInstance is accessed via std::atomic_load/atomic_store rather than
// a plain pointer: every static method here first takes its own local
// shared_ptr snapshot, so the Logger object it calls into stays alive for
// the duration of that call even if another thread concurrently calls
// setLogger()/shutdown() and drops the last other reference to it.

void Logger::shutdown() {
   std::atomic_store(&loggerInstance, std::shared_ptr<Logger>());
}

//******************************************************************************

void Logger::setLogger(Logger* logger) {
   std::atomic_store(&loggerInstance, std::shared_ptr<Logger>(logger));
}

//******************************************************************************

Logger* Logger::getLogger() {
   return std::atomic_load(&loggerInstance).get();
}

//******************************************************************************

void Logger::log(LogLevel logLevel, const std::string& logMessage) {
   const std::shared_ptr<Logger> instance = std::atomic_load(&loggerInstance);
   if (instance) {
      instance->logMessage(logLevel, logMessage);
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
   const std::shared_ptr<Logger> instance = std::atomic_load(&loggerInstance);
   if (instance) {
      return instance->isLoggingLevel(logLevel);
   }

   return false;
}

//******************************************************************************

void Logger::logInstanceCreate(const char* className) {
   const std::shared_ptr<Logger> instance = std::atomic_load(&loggerInstance);
   if (instance) {
      if (instance->isLoggingInstanceLifecycles()) {
         instance->logInstanceCreate(std::string(className));
      }
   }
}

//******************************************************************************

void Logger::logInstanceDestroy(const char* className) {
   const std::shared_ptr<Logger> instance = std::atomic_load(&loggerInstance);
   if (instance) {
      if (instance->isLoggingInstanceLifecycles()) {
         instance->logInstanceDestroy(std::string(className));
      }
   }
}

//******************************************************************************

void Logger::countOccurrence(const char* occurrenceType,
                             const char* occurrenceName) {
   const std::shared_ptr<Logger> instance = std::atomic_load(&loggerInstance);
   if (instance) {
      instance->logOccurrence(std::string(occurrenceType),
                              std::string(occurrenceName));
   }
}

//******************************************************************************

void Logger::countOccurrence(const std::string& occurrenceType,
                             const std::string& occurrenceName) {
   const std::shared_ptr<Logger> instance = std::atomic_load(&loggerInstance);
   if (instance) {
      instance->logOccurrence(occurrenceType, occurrenceName);
   }
}

//******************************************************************************

