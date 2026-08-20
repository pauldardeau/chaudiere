// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include <unistd.h>

#include "TestOSUtils.h"
#include "OSUtils.h"

using namespace chaudiere;

//******************************************************************************

TestOSUtils::TestOSUtils() :
   poivre::TestSuite("TestOSUtils") {
}

//******************************************************************************

void TestOSUtils::runTests() {
   testGetCurrentDirectory();
   testDirectoryExists();
   testPathExists();
   testPathJoin();
   testCreateDirectory();
   testCreatePrivateDirectory();
   testGetFileSize();
   testDeleteFile();
   testRenameFile();
   testSysPlatform();
   testOsName();
   testSplitExt();
   testCurrentTimeMillis();
   testListFilesInDirectory();
   testListDirsInDirectory();
   testCrc32ForBuffer();
   testCrc32ForFile();
   testGetOSHostName();
   testGetOSUser();
   testGetOneMinuteLoadAvg();
   testGetOSName();
   testGetOSRelease();
   testGetHWCpuCount();
   testGetOSCurrentTimestamp();
}

//******************************************************************************

void TestOSUtils::testGetCurrentDirectory() {
   TEST_CASE("testGetCurrentDirectory");

   requireNonEmptyString(OSUtils::getCurrentDirectory(), "getCurrentDirectory should return a non-empty path");
}

//******************************************************************************

void TestOSUtils::testDirectoryExists() {
   TEST_CASE("testDirectoryExists");

   require(OSUtils::directoryExists(OSUtils::getCurrentDirectory()), "the current directory should exist");
   requireFalse(OSUtils::directoryExists("/no/such/directory/xyz"), "a non-existent directory should not exist");
}

//******************************************************************************

void TestOSUtils::testPathExists() {
   TEST_CASE("testPathExists");

   const std::string filePath = getTempFile();
   require(OSUtils::pathExists(filePath), "a freshly created temp file should exist");
   deleteFile(filePath);
   requireFalse(OSUtils::pathExists(filePath), "the path should no longer exist after deletion");
}

//******************************************************************************

void TestOSUtils::testPathJoin() {
   TEST_CASE("testPathJoin");

   requireStringEquals("/tmp/file.txt", OSUtils::pathJoin("/tmp", "file.txt"), "pathJoin should insert a separator when the directory doesn't end with one");
   requireStringEquals("/tmp/file.txt", OSUtils::pathJoin("/tmp/", "file.txt"), "pathJoin should not double the separator when the directory already ends with one");
}

//******************************************************************************

void TestOSUtils::testCreateDirectory() {
   TEST_CASE("testCreateDirectory");

   const std::string dirPath = getTempFile();
   deleteFile(dirPath); // free up the path so it can be used as a directory

   require(OSUtils::createDirectory(dirPath), "createDirectory should succeed for a new directory path");
   require(OSUtils::directoryExists(dirPath), "directoryExists should be true after createDirectory");

   ::rmdir(dirPath.c_str());
}

//******************************************************************************

void TestOSUtils::testCreatePrivateDirectory() {
   TEST_CASE("testCreatePrivateDirectory");

   const std::string dirPath = getTempFile();
   deleteFile(dirPath);

   require(OSUtils::createPrivateDirectory(dirPath), "createPrivateDirectory should succeed for a new directory path");
   require(OSUtils::directoryExists(dirPath), "directoryExists should be true after createPrivateDirectory");

   ::rmdir(dirPath.c_str());
}

//******************************************************************************

void TestOSUtils::testGetFileSize() {
   TEST_CASE("testGetFileSize");

   const std::string filePath = getTempFile();
   FILE* f = ::fopen(filePath.c_str(), "wb");
   require(nullptr != f, "sanity check: temp file should be writable");
   ::fwrite("12345", 1, 5, f);
   ::fclose(f);

   require(5L == OSUtils::getFileSize(filePath), "getFileSize should report the number of bytes written");
   require(-1L == OSUtils::getFileSize("/no/such/path/xyz"), "getFileSize should return -1 for a non-existent path");

   deleteFile(filePath);
}

//******************************************************************************

void TestOSUtils::testDeleteFile() {
   TEST_CASE("testDeleteFile");

   const std::string filePath = getTempFile();
   require(OSUtils::pathExists(filePath), "sanity check: temp file should exist before deletion");
   require(OSUtils::deleteFile(filePath), "deleteFile should succeed for an existing file");
   requireFalse(OSUtils::pathExists(filePath), "the file should no longer exist after deleteFile");
}

//******************************************************************************

void TestOSUtils::testRenameFile() {
   TEST_CASE("testRenameFile");

   const std::string oldPath = getTempFile();
   const std::string newPath = oldPath + ".renamed";

   require(OSUtils::renameFile(oldPath, newPath), "renameFile should succeed for an existing file");
   requireFalse(OSUtils::pathExists(oldPath), "the old path should no longer exist after rename");
   require(OSUtils::pathExists(newPath), "the new path should exist after rename");

   deleteFile(newPath);
}

//******************************************************************************

void TestOSUtils::testSysPlatform() {
   TEST_CASE("testSysPlatform");

#ifdef __linux__
   requireStringEquals("linux", OSUtils::sysPlatform(), "sysPlatform should report 'linux' on this platform");
#else
   requireNonEmptyString(OSUtils::sysPlatform(), "sysPlatform should return a non-empty platform name");
#endif
}

//******************************************************************************

void TestOSUtils::testOsName() {
   TEST_CASE("testOsName");

   requireStringEquals("posix", OSUtils::osName(), "osName should report 'posix' on this platform");
}

//******************************************************************************

void TestOSUtils::testSplitExt() {
   TEST_CASE("testSplitExt");

   std::vector<std::string> parts1;
   OSUtils::splitExt("/path/to/file.txt", parts1);
   require(2 == parts1.size(), "splitExt should produce root and extension parts");
   requireStringEquals("/path/to/file", parts1[0], "root part should exclude the extension");
   requireStringEquals(".txt", parts1[1], "extension part should include the leading dot");

   std::vector<std::string> parts2;
   OSUtils::splitExt("/path/to/noext", parts2);
   require(2 == parts2.size(), "splitExt should still produce two parts when there's no extension");
   requireStringEquals("/path/to/noext", parts2[0], "root part should be the whole path when there's no extension");
   require(parts2[1].empty(), "extension part should be empty when there's no extension");
}

//******************************************************************************

void TestOSUtils::testCurrentTimeMillis() {
   TEST_CASE("testCurrentTimeMillis");

   // documented TODO in OSUtils.cpp: currentTimeMillis() is unimplemented
   // and always returns 0
   require(0L == OSUtils::currentTimeMillis(), "currentTimeMillis is unimplemented and currently always returns 0");
}

//******************************************************************************

void TestOSUtils::testListFilesInDirectory() {
   TEST_CASE("testListFilesInDirectory");

   const std::string dirPath = getTempFile();
   deleteFile(dirPath);
   OSUtils::createDirectory(dirPath);

   const std::string filePath = OSUtils::pathJoin(dirPath, "afile.txt");
   FILE* f = ::fopen(filePath.c_str(), "w");
   if (nullptr != f) {
      ::fclose(f);
   }

   const std::string subDirPath = OSUtils::pathJoin(dirPath, "asubdir");
   OSUtils::createDirectory(subDirPath);

   std::vector<std::string> files = OSUtils::listFilesInDirectory(dirPath);
   require(1 == files.size(), "listFilesInDirectory should list only the file, not the subdirectory");
   if (1 == files.size()) {
      requireStringEquals("afile.txt", files[0], "listed file name should match");
   }

   ::rmdir(subDirPath.c_str());
   ::unlink(filePath.c_str());
   ::rmdir(dirPath.c_str());
}

//******************************************************************************

void TestOSUtils::testListDirsInDirectory() {
   TEST_CASE("testListDirsInDirectory");

   const std::string dirPath = getTempFile();
   deleteFile(dirPath);
   OSUtils::createDirectory(dirPath);

   const std::string filePath = OSUtils::pathJoin(dirPath, "afile.txt");
   FILE* f = ::fopen(filePath.c_str(), "w");
   if (nullptr != f) {
      ::fclose(f);
   }

   const std::string subDirPath = OSUtils::pathJoin(dirPath, "asubdir");
   OSUtils::createDirectory(subDirPath);

   std::vector<std::string> dirs = OSUtils::listDirsInDirectory(dirPath);
   require(1 == dirs.size(), "listDirsInDirectory should list only the subdirectory, not the file");
   if (1 == dirs.size()) {
      requireStringEquals("asubdir", dirs[0], "listed directory name should match");
   }

   ::rmdir(subDirPath.c_str());
   ::unlink(filePath.c_str());
   ::rmdir(dirPath.c_str());
}

//******************************************************************************

void TestOSUtils::testCrc32ForBuffer() {
   TEST_CASE("testCrc32ForBuffer");

   // "123456789" is the standard CRC-32/ISO-HDLC check value input; the
   // expected result (0xCBF43926) is a well-known test vector for this
   // exact algorithm (reflected CRC-32, poly 0xEDB88320, init/final XOR
   // 0xFFFFFFFF)
   const std::string data = "123456789";
   const unsigned long crc = OSUtils::crc32ForBuffer(0, data.data(), data.size());
   require(0xCBF43926UL == crc, "crc32ForBuffer should match the standard CRC-32 check value for '123456789'");
}

//******************************************************************************

void TestOSUtils::testCrc32ForFile() {
   TEST_CASE("testCrc32ForFile");

   const std::string filePath = getTempFile();
   FILE* f = ::fopen(filePath.c_str(), "wb");
   require(nullptr != f, "sanity check: temp file should be writable");
   ::fwrite("123456789", 1, 9, f);
   ::fclose(f);

   std::string crc;
   require(OSUtils::crc32ForFile(filePath, crc), "crc32ForFile should succeed for an existing, readable file");
   requireStringEquals("cbf43926", crc, "crc32ForFile should match the standard CRC-32 check value for '123456789'");

   deleteFile(filePath);
}

//******************************************************************************

void TestOSUtils::testGetOSHostName() {
   TEST_CASE("testGetOSHostName");

   std::string hostName;
   require(OSUtils::getOSHostName(hostName), "getOSHostName should succeed on this platform");
   requireNonEmptyString(hostName, "host name should be non-empty");
}

//******************************************************************************

void TestOSUtils::testGetOSUser() {
   TEST_CASE("testGetOSUser");

   std::string user;
   require(OSUtils::getOSUser(user), "getOSUser should succeed on this platform");
   requireNonEmptyString(user, "user name should be non-empty");
}

//******************************************************************************

void TestOSUtils::testGetOneMinuteLoadAvg() {
   TEST_CASE("testGetOneMinuteLoadAvg");

   require(OSUtils::getOneMinuteLoadAvg() >= 0.0, "one-minute load average should be non-negative");
}

//******************************************************************************

void TestOSUtils::testGetOSName() {
   TEST_CASE("testGetOSName");

   std::string osName;
   require(OSUtils::getOSName(osName), "getOSName should succeed on this platform");
   requireNonEmptyString(osName, "OS name should be non-empty");
}

//******************************************************************************

void TestOSUtils::testGetOSRelease() {
   TEST_CASE("testGetOSRelease");

   std::string osRelease;
   require(OSUtils::getOSRelease(osRelease), "getOSRelease should succeed on this platform");
   requireNonEmptyString(osRelease, "OS release should be non-empty");
}

//******************************************************************************

void TestOSUtils::testGetHWCpuCount() {
   TEST_CASE("testGetHWCpuCount");

   int count = 0;
#ifdef __linux__
   require(OSUtils::getHWCpuCount(count), "getHWCpuCount should succeed on Linux");
   require(count > 0, "CPU count should be positive");
#else
   OSUtils::getHWCpuCount(count);
   require(true, "getHWCpuCount should not crash on other platforms");
#endif
}

//******************************************************************************

void TestOSUtils::testGetOSCurrentTimestamp() {
   TEST_CASE("testGetOSCurrentTimestamp");

   std::string timestamp;
   require(OSUtils::getOSCurrentTimestamp(timestamp), "getOSCurrentTimestamp should succeed");
   require(19 == timestamp.length(), "timestamp should be formatted as 'yyyy-mm-dd hh:mm:ss' (19 characters)");
}

//******************************************************************************
