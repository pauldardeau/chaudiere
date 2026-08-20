// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <unordered_map>

#include "TestStdLogger.h"
#include "StdLogger.h"

using namespace chaudiere;

//******************************************************************************

TestStdLogger::TestStdLogger() :
   poivre::TestSuite("TestStdLogger") {
}

//******************************************************************************

void TestStdLogger::tearDown() {
   // Logger's installed instance is process-global state; reset it after
   // every test case so tests (and other suites) don't see a stale logger
   Logger::shutdown();
}

//******************************************************************************

void TestStdLogger::runTests() {
   testConstructor();
   testConstructorWithLogLevel();
   testGetLogLevel();
   testSetLogLevel();
   testIsLoggingLevel();
   testLogLevelPrefix();
   testIsLoggingInstanceLifecycles();
   testSetLogInstanceLifecycles();
   testLogInstanceCreateAndDestroy();
   testLogOccurrence();
   testLogMessage();
}

//******************************************************************************

void TestStdLogger::testConstructor() {
   TEST_CASE("testConstructor");

   StdLogger logger;
   require(Debug == logger.getLogLevel(), "default log level should be Debug");
}

//******************************************************************************

void TestStdLogger::testConstructorWithLogLevel() {
   TEST_CASE("testConstructorWithLogLevel");

   StdLogger logger(Warning);
   require(Warning == logger.getLogLevel(), "log level should match the constructor argument");
}

//******************************************************************************

void TestStdLogger::testGetLogLevel() {
   TEST_CASE("testGetLogLevel");

   StdLogger logger(Error);
   require(Error == logger.getLogLevel(), "getLogLevel should return the current log level");
}

//******************************************************************************

void TestStdLogger::testSetLogLevel() {
   TEST_CASE("testSetLogLevel");

   StdLogger logger;
   logger.setLogLevel(Verbose);
   require(Verbose == logger.getLogLevel(), "getLogLevel should reflect setLogLevel");
}

//******************************************************************************

void TestStdLogger::testIsLoggingLevel() {
   TEST_CASE("testIsLoggingLevel");

   StdLogger logger(Warning);
   require(logger.isLoggingLevel(Critical), "less verbose levels than the current level should be logged");
   require(logger.isLoggingLevel(Warning), "the current level itself should be logged");
   requireFalse(logger.isLoggingLevel(Info), "more verbose levels than the current level should not be logged");
   requireFalse(logger.isLoggingLevel(Debug), "more verbose levels than the current level should not be logged");
}

//******************************************************************************

void TestStdLogger::testLogLevelPrefix() {
   TEST_CASE("testLogLevelPrefix");

   StdLogger logger;
   requireStringEquals("Critical:", logger.logLevelPrefix(Critical), "prefix for Critical");
   requireStringEquals("Error:", logger.logLevelPrefix(Error), "prefix for Error");
   requireStringEquals("Warning:", logger.logLevelPrefix(Warning), "prefix for Warning");
   requireStringEquals("Info:", logger.logLevelPrefix(Info), "prefix for Info");
   requireStringEquals("Debug:", logger.logLevelPrefix(Debug), "prefix for Debug");
   requireStringEquals("Verbose:", logger.logLevelPrefix(Verbose), "prefix for Verbose");
}

//******************************************************************************

void TestStdLogger::testIsLoggingInstanceLifecycles() {
   TEST_CASE("testIsLoggingInstanceLifecycles");

   StdLogger logger;
   requireFalse(logger.isLoggingInstanceLifecycles(), "instance lifecycle logging should default to false");
}

//******************************************************************************

void TestStdLogger::testSetLogInstanceLifecycles() {
   TEST_CASE("testSetLogInstanceLifecycles");

   StdLogger logger;
   logger.setLogInstanceLifecycles(true);
   require(logger.isLoggingInstanceLifecycles(), "isLoggingInstanceLifecycles should reflect setLogInstanceLifecycles(true), unlike FileLogger's no-op version");
   logger.setLogInstanceLifecycles(false);
   requireFalse(logger.isLoggingInstanceLifecycles(), "isLoggingInstanceLifecycles should reflect setLogInstanceLifecycles(false)");
}

//******************************************************************************

void TestStdLogger::testLogInstanceCreateAndDestroy() {
   TEST_CASE("testLogInstanceCreateAndDestroy");

   StdLogger logger;
   logger.logInstanceCreate("SomeClass");
   logger.logInstanceCreate("SomeClass");
   logger.logInstanceDestroy("SomeClass");

   std::unordered_map<std::string, LifecycleStats> stats;
   logger.populateClassLifecycleStats(stats);

   require(stats.count("SomeClass") > 0, "lifecycle stats should track a class after logInstanceCreate");
   require(2 == stats["SomeClass"].m_instancesCreated, "instancesCreated should reflect the number of logInstanceCreate calls");
   require(1 == stats["SomeClass"].m_instancesDestroyed, "instancesDestroyed should reflect the number of logInstanceDestroy calls");
}

//******************************************************************************

void TestStdLogger::testLogOccurrence() {
   TEST_CASE("testLogOccurrence");

   StdLogger logger;
   logger.logOccurrence("eventType", "eventName");
   logger.logOccurrence("eventType", "eventName");
   logger.logOccurrence("eventType", "otherName");

   std::unordered_map<std::string, std::unordered_map<std::string, long long>> occurrences;
   logger.populateOccurrences(occurrences);

   require(occurrences.count("eventType") > 0, "occurrences should track the given event type");
   require(2 == occurrences["eventType"]["eventName"], "occurrence count should reflect the number of logOccurrence calls for that name");
   require(1 == occurrences["eventType"]["otherName"], "a different event name under the same type should be tracked separately");
}

//******************************************************************************

void TestStdLogger::testLogMessage() {
   TEST_CASE("testLogMessage");

   // logMessage() filters via the static Logger::isLogging(), which checks
   // whatever instance is currently installed via Logger::setLogger() --
   // exercise it through the installed singleton rather than calling
   // logMessage() directly on a standalone instance (see the identical
   // note in TestFileLogger).
   Logger::setLogger(new StdLogger(Debug));
   Logger::critical("a critical message");
   require(true, "logMessage (invoked via the installed singleton) should not throw");
}

//******************************************************************************
