// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <fstream>
#include <string>

#include "TestFileLogger.h"
#include "FileLogger.h"
#include "StrUtils.h"

using namespace chaudiere;

//******************************************************************************

TestFileLogger::TestFileLogger() :
   poivre::TestSuite("TestFileLogger") {
}

//******************************************************************************

void TestFileLogger::tearDown() {
   // Logger's installed instance is process-global state; reset it after
   // every test case so tests (and other suites) don't see a stale logger
   Logger::shutdown();
}

//******************************************************************************

void TestFileLogger::runTests() {
   testConstructor();
   testConstructorWithLogLevel();
   testGetLogLevel();
   testSetLogLevel();
   testIsLoggingLevel();
   testLogLevelPrefix();
   testIsLoggingInstanceLifecycles();
   testSetLogInstanceLifecycles();
   testLogInstanceCreate();
   testLogInstanceDestroy();
   testLogOccurrence();
   testLogMessage();
   testSetLoggerAndGetLogger();
   testShutdown();
   testIsLogging();
}

//******************************************************************************

void TestFileLogger::testConstructor() {
   TEST_CASE("testConstructor");

   FileLogger logger(getTempFile());
   require(Debug == logger.getLogLevel(), "default log level should be Debug");
}

//******************************************************************************

void TestFileLogger::testConstructorWithLogLevel() {
   TEST_CASE("testConstructorWithLogLevel");

   FileLogger logger(getTempFile(), Warning);
   require(Warning == logger.getLogLevel(), "log level should match the constructor argument");
}

//******************************************************************************

void TestFileLogger::testGetLogLevel() {
   TEST_CASE("testGetLogLevel");

   FileLogger logger(getTempFile(), Error);
   require(Error == logger.getLogLevel(), "getLogLevel should return the current log level");
}

//******************************************************************************

void TestFileLogger::testSetLogLevel() {
   TEST_CASE("testSetLogLevel");

   FileLogger logger(getTempFile());
   logger.setLogLevel(Verbose);
   require(Verbose == logger.getLogLevel(), "getLogLevel should reflect setLogLevel");
}

//******************************************************************************

void TestFileLogger::testIsLoggingLevel() {
   TEST_CASE("testIsLoggingLevel");

   FileLogger logger(getTempFile(), Warning);
   require(logger.isLoggingLevel(Critical), "less verbose levels than the current level should be logged");
   require(logger.isLoggingLevel(Error), "less verbose levels than the current level should be logged");
   require(logger.isLoggingLevel(Warning), "the current level itself should be logged");
   requireFalse(logger.isLoggingLevel(Info), "more verbose levels than the current level should not be logged");
   requireFalse(logger.isLoggingLevel(Debug), "more verbose levels than the current level should not be logged");
}

//******************************************************************************

void TestFileLogger::testLogLevelPrefix() {
   TEST_CASE("testLogLevelPrefix");

   FileLogger logger(getTempFile());
   requireStringEquals("Critical:", logger.logLevelPrefix(Critical), "prefix for Critical");
   requireStringEquals("Error:", logger.logLevelPrefix(Error), "prefix for Error");
   requireStringEquals("Warning:", logger.logLevelPrefix(Warning), "prefix for Warning");
   requireStringEquals("Info:", logger.logLevelPrefix(Info), "prefix for Info");
   requireStringEquals("Debug:", logger.logLevelPrefix(Debug), "prefix for Debug");
   requireStringEquals("Verbose:", logger.logLevelPrefix(Verbose), "prefix for Verbose");
}

//******************************************************************************

void TestFileLogger::testIsLoggingInstanceLifecycles() {
   TEST_CASE("testIsLoggingInstanceLifecycles");

   FileLogger logger(getTempFile());
   requireFalse(logger.isLoggingInstanceLifecycles(), "FileLogger does not support instance lifecycle logging");
}

//******************************************************************************

void TestFileLogger::testSetLogInstanceLifecycles() {
   TEST_CASE("testSetLogInstanceLifecycles");

   FileLogger logger(getTempFile());
   logger.setLogInstanceLifecycles(true);
   requireFalse(logger.isLoggingInstanceLifecycles(), "setLogInstanceLifecycles is a no-op on FileLogger");
}

//******************************************************************************

void TestFileLogger::testLogInstanceCreate() {
   TEST_CASE("testLogInstanceCreate");

   FileLogger logger(getTempFile());
   logger.logInstanceCreate("SomeClass");
   require(true, "logInstanceCreate should not throw (it's a no-op on FileLogger)");
}

//******************************************************************************

void TestFileLogger::testLogInstanceDestroy() {
   TEST_CASE("testLogInstanceDestroy");

   FileLogger logger(getTempFile());
   logger.logInstanceDestroy("SomeClass");
   require(true, "logInstanceDestroy should not throw (it's a no-op on FileLogger)");
}

//******************************************************************************

void TestFileLogger::testLogOccurrence() {
   TEST_CASE("testLogOccurrence");

   FileLogger logger(getTempFile());
   logger.logOccurrence("someType", "someName");
   require(true, "logOccurrence should not throw (it's a no-op on FileLogger)");
}

//******************************************************************************

void TestFileLogger::testLogMessage() {
   TEST_CASE("testLogMessage");

   // logMessage() filters via the static Logger::isLogging(), which checks
   // whatever instance is currently installed via Logger::setLogger() --
   // not necessarily "this" -- so exercising it for real means going
   // through the installed singleton rather than calling logMessage()
   // directly on a standalone instance.
   const std::string logPath = getTempFile();
   Logger::setLogger(new FileLogger(logPath, Debug));

   Logger::critical("a critical message");
   Logger::shutdown(); // drops the last reference, closing the file

   std::ifstream logFile(logPath.c_str());
   std::string line;
   require((bool) std::getline(logFile, line), "log file should contain at least one line");
   require(StrUtils::containsString(line, "Critical:"), "logged line should contain the level prefix");
   require(StrUtils::containsString(line, "a critical message"), "logged line should contain the message");

   deleteFile(logPath);
}

//******************************************************************************

void TestFileLogger::testSetLoggerAndGetLogger() {
   TEST_CASE("testSetLoggerAndGetLogger");

   require(nullptr == Logger::getLogger(), "getLogger should be null before any logger has been installed");

   FileLogger* fileLogger = new FileLogger(getTempFile());
   Logger::setLogger(fileLogger);
   require(fileLogger == Logger::getLogger(), "getLogger should return the instance passed to setLogger");
}

//******************************************************************************

void TestFileLogger::testShutdown() {
   TEST_CASE("testShutdown");

   Logger::setLogger(new FileLogger(getTempFile()));
   require(nullptr != Logger::getLogger(), "sanity check: a logger should be installed");

   Logger::shutdown();
   require(nullptr == Logger::getLogger(), "getLogger should be null after shutdown");
}

//******************************************************************************

void TestFileLogger::testIsLogging() {
   TEST_CASE("testIsLogging");

   requireFalse(Logger::isLogging(Critical), "isLogging should be false with no logger installed");

   Logger::setLogger(new FileLogger(getTempFile(), Warning));
   require(Logger::isLogging(Warning), "isLogging should reflect the installed logger's level");
   requireFalse(Logger::isLogging(Debug), "isLogging should be false for levels more verbose than the installed logger's level");
}

//******************************************************************************
