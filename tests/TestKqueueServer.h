// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTKQUEUESERVER_H
#define CHAUDIERE_TESTKQUEUESERVER_H

#include "TestSuite.h"

namespace chaudiere
{

class TestKqueueServer : public poivre::TestSuite
{
protected:
   void runTests();

   void testIsSupportedPlatform();
   void testConstructor();
   void testInit();
   void testRun();
   void testNotifySocketComplete();
   void testUnsupportedPlatformBehavior();
   void testUnsupportedInitWithValidHandler();

   // run instead of testInit() and the unsupported-platform tests above
   // where kqueue is supported
   void testSupportedInit();
   void testSupportedFileDescriptorRegistration();
   void testSupportedReadableDescriptorEvent();
   void testSupportedListenerEvent();
   void testSupportedEndOfStreamEvent();
   void testSupportedRemovedDescriptorIsNotReported();

public:
   TestKqueueServer();


};

}

#endif
