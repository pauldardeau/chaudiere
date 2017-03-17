// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_LOGGER_H
#define CHAUDIERE_LOGGER_H

#include <string>
#include <memory>

namespace chaudiere
{

enum LogLevel
{
   Critical = 0,  // least verbose
   Error    = 1,
   Warning  = 2,
   Info     = 3,
   Debug    = 4,
   Verbose  = 5   // most verbose
};

/**
 * Logger is an interface (abstract base class) for logging.
 */
class Logger
{
public:
   /**
    * Destructor
    */
   virtual ~Logger() {}
   
   /**
    * Retrieves the current log level
    * @return the current log level value
    */
   virtual LogLevel getLogLevel() const = 0;
   
   /**
    * Sets the current log level
    * @param logLevel the new log level
    */
   virtual void setLogLevel(LogLevel logLevel) = 0;
   
   /**
    *
    * @return
    */
   virtual bool isLoggingInstanceLifecycles() const = 0;
   
   /**
    *
    * @param logInstanceLifecycles
    */
   virtual void setLogInstanceLifecycles(bool logInstanceLifecycles) = 0;
   
   /**
    *
    * @param className
    */
   virtual void logInstanceCreate(const std::string& className) = 0;
   
   /**
    *
    * @param className
    */
   virtual void logInstanceDestroy(const std::string& className) = 0;
   
   /**
    *
    * @param logLevel
    * @param logMessage
    */
   virtual void logMessage(LogLevel logLevel, const std::string& logMessage) = 0;
   
   /**
    *
    * @param logLevel
    * @return
    */
   virtual bool isLoggingLevel(LogLevel logLevel) const = 0;
   
   /**
    *
    * @param occurrenceType
    * @param occurrenceName
    */
   virtual void logOccurrence(const std::string& occurrenceType,
                              const std::string& occurrenceName) = 0;
   
   /**
    *
    */
   static void shutdown();
                              
   /**
    *
    * @param logger
    */
   static void setLogger(Logger* logger);
   
   /**
    *
    * @return
    */
   static Logger* getLogger();
   
   /**
    *
    * @param logLevel
    * @param logMessage
    */
   static void log(LogLevel logLevel, const std::string& logMessage);
   
   /**
    *
    * @param logMessage
    */
   static void critical(const std::string& logMessage);
   
  /**
   *
   * @param logMessage
   */
   static void error(const std::string& logMessage);
   
  /**
   *
   * @param logMessage
   */
   static void warning(const std::string& logMessage);
   
  /**
   *
   * @param logMessage
   */
   static void info(const std::string& logMessage);
   
  /**
   *
   * @param logMessage
   */
   static void debug(const std::string& logMessage);
   
  /**
   *
   * @param logMessage
   */
   static void verbose(const std::string& logMessage);
   
   /**
    *
    * @param logLevel
    * @return
    */
   static bool isLogging(LogLevel logLevel);
   
   // object lifecycles (create/destroy objects)
   /**
    *
    * @param className
    */
   static void logInstanceCreate(const char* className);
   
   /**
    *
    * @param className
    */
   static void logInstanceDestroy(const char* className);

   // counting occurrences
   /**
    *
    * @param occurrenceType
    * @param occurrence
    */
   static void countOccurrence(const char* occurrenceType,
                               const char* occurrence);
                               
   /**
    *
    * @param occurrenceType
    * @param occurrence
    */
   static void countOccurrence(const std::string& occurrenceType,
                               const std::string& occurrence);
   
   
private:
   static Logger* loggerInstance;
   
};

}

#endif
