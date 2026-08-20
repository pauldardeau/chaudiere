// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSTDLOGGER_H
#define CHAUDIERE_TESTSTDLOGGER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestStdLogger : public poivre::TestSuite
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
   void testLogInstanceCreateAndDestroy();
   void testLogOccurrence();
   void testLogMessage();

public:
   TestStdLogger();

};

}

#endif
