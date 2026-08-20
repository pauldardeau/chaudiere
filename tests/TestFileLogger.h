// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTFILELOGGER_H
#define CHAUDIERE_TESTFILELOGGER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestFileLogger : public poivre::TestSuite
{
protected:
   void runTests();
   void tearDown();

   void testConstructor();
   void testConstructorWithLogLevel();
   void testGetLogLevel();
   void testSetLogLevel();
   void testIsLoggingLevel();
   void testLogLevelPrefix();
   void testIsLoggingInstanceLifecycles();
   void testSetLogInstanceLifecycles();
   void testLogInstanceCreate();
   void testLogInstanceDestroy();
   void testLogOccurrence();
   void testLogMessage();
   void testSetLoggerAndGetLogger();
   void testShutdown();
   void testIsLogging();

public:
   TestFileLogger();

};

}

#endif
