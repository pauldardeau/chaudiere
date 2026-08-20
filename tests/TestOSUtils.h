// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTOSUTILS_H
#define CHAUDIERE_TESTOSUTILS_H

#include "TestSuite.h"

namespace chaudiere
{

class TestOSUtils : public poivre::TestSuite
{
protected:
   void runTests();

   void testGetCurrentDirectory();
   void testDirectoryExists();
   void testPathExists();
   void testPathJoin();
   void testCreateDirectory();
   void testCreatePrivateDirectory();
   void testGetFileSize();
   void testDeleteFile();
   void testRenameFile();
   void testSysPlatform();
   void testOsName();
   void testSplitExt();
   void testCurrentTimeMillis();
   void testListFilesInDirectory();
   void testListDirsInDirectory();
   void testCrc32ForBuffer();
   void testCrc32ForFile();
   void testGetOSHostName();
   void testGetOSUser();
   void testGetOneMinuteLoadAvg();
   void testGetOSName();
   void testGetOSRelease();
   void testGetHWCpuCount();
   void testGetOSCurrentTimestamp();

public:
   TestOSUtils();

};

}

#endif
