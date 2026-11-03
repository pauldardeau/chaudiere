// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include <filesystem>
#include <limits>
#include <system_error>

#include "TestOSUtils.h"
#include "OSUtils.h"

using namespace chaudiere;

//******************************************************************************

// removes a file or an (empty) directory, whichever the path is
static void removePath(const std::string& path) {
   std::error_code ec;
   std::filesystem::remove(std::filesystem::path{path}, ec);
}

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
   testGetFileSizeBeyondLong();
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
   testUnsupportedOperations();
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

   const std::string filePath = getTempFile();
   require(OSUtils::pathExists(filePath), "sanity check: temp file should exist");
   requireFalse(OSUtils::directoryExists(filePath), "a regular file is not a directory");
   deleteFile(filePath);
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

#ifdef _WIN32
   // the separator is a backslash, but a directory ending in either kind
   // already ends with one
   requireStringEquals("C:\\tmp\\file.txt", OSUtils::pathJoin("C:\\tmp", "file.txt"), "pathJoin should insert a backslash when the directory doesn't end with one");
   requireStringEquals("C:\\tmp\\file.txt", OSUtils::pathJoin("C:\\tmp\\", "file.txt"), "pathJoin should not double the backslash when the directory already ends with one");
   requireStringEquals("C:/tmp/file.txt", OSUtils::pathJoin("C:/tmp/", "file.txt"), "pathJoin should treat a trailing forward slash as a separator on Windows");
   requireStringEquals("\\file.txt", OSUtils::pathJoin("", "file.txt"), "pathJoin of an empty directory starts at the root, as on POSIX");
#else
   requireStringEquals("/tmp/file.txt", OSUtils::pathJoin("/tmp", "file.txt"), "pathJoin should insert a separator when the directory doesn't end with one");
   requireStringEquals("/tmp/file.txt", OSUtils::pathJoin("/tmp/", "file.txt"), "pathJoin should not double the separator when the directory already ends with one");
#endif
}

//******************************************************************************

void TestOSUtils::testCreateDirectory() {
   TEST_CASE("testCreateDirectory");

   const std::string dirPath = getTempFile();
   deleteFile(dirPath); // free up the path so it can be used as a directory

   require(OSUtils::createDirectory(dirPath), "createDirectory should succeed for a new directory path");
   require(OSUtils::directoryExists(dirPath), "directoryExists should be true after createDirectory");
   requireFalse(OSUtils::createDirectory(dirPath), "createDirectory should fail when the directory already exists");

   const std::string orphanPath = OSUtils::pathJoin(OSUtils::pathJoin(dirPath, "missing"), "child");
   requireFalse(OSUtils::createDirectory(orphanPath), "createDirectory should fail when the parent doesn't exist");
   requireFalse(OSUtils::directoryExists(orphanPath), "a failed createDirectory should leave nothing behind");

   removePath(dirPath);
}

//******************************************************************************

void TestOSUtils::testCreatePrivateDirectory() {
   TEST_CASE("testCreatePrivateDirectory");

   const std::string dirPath = getTempFile();
   deleteFile(dirPath);

#ifdef _WIN32
   // not supported on Windows: it must neither create a directory with
   // default (inherited) permissions nor claim to have made a private one
   requireFalse(OSUtils::createPrivateDirectory(dirPath), "createPrivateDirectory should report failure on Windows");
   requireFalse(OSUtils::directoryExists(dirPath), "createPrivateDirectory should not create anything on Windows");
#else
   require(OSUtils::createPrivateDirectory(dirPath), "createPrivateDirectory should succeed for a new directory path");
   require(OSUtils::directoryExists(dirPath), "directoryExists should be true after createPrivateDirectory");
#endif

   removePath(dirPath);
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

void TestOSUtils::testGetFileSizeBeyondLong() {
   TEST_CASE("testGetFileSizeBeyondLong");

   // only where long is narrower than the file sizes the filesystem allows
   // (Windows, LLP64): a size that doesn't fit must come back as the -1 failure
   // value, not wrapped around into some other number
   if constexpr (sizeof(long) < sizeof(std::uintmax_t)) {
      const std::uintmax_t largest = static_cast<std::uintmax_t>(std::numeric_limits<long>::max());
      const std::string filePath = getTempFile();
      const std::filesystem::path path{filePath};
      std::error_code ec;

      std::filesystem::resize_file(path, largest, ec);
      require(!ec, "sanity check: the temp file can be sized to the largest long");
      if (!ec) {
         require(std::numeric_limits<long>::max() == OSUtils::getFileSize(filePath), "the largest size a long holds should be reported as is");
      }

      std::filesystem::resize_file(path, largest + 1, ec);
      require(!ec, "sanity check: the temp file can be sized one past the largest long");
      if (!ec) {
         require(-1L == OSUtils::getFileSize(filePath), "a size too large for a long should be reported as -1");
      }

      deleteFile(filePath);
   } else {
      require(true, "long holds every file size on this platform; nothing to check");
   }
}

//******************************************************************************

void TestOSUtils::testDeleteFile() {
   TEST_CASE("testDeleteFile");

   const std::string filePath = getTempFile();
   require(OSUtils::pathExists(filePath), "sanity check: temp file should exist before deletion");
   require(OSUtils::deleteFile(filePath), "deleteFile should succeed for an existing file");
   requireFalse(OSUtils::pathExists(filePath), "the file should no longer exist after deleteFile");
   requireFalse(OSUtils::deleteFile(filePath), "deleteFile should fail for a file that no longer exists");
}

//******************************************************************************

void TestOSUtils::testRenameFile() {
   TEST_CASE("testRenameFile");

   const std::string oldPath = getTempFile();
   const std::string newPath = oldPath + ".renamed";

   require(OSUtils::renameFile(oldPath, newPath), "renameFile should succeed for an existing file");
   requireFalse(OSUtils::pathExists(oldPath), "the old path should no longer exist after rename");
   require(OSUtils::pathExists(newPath), "the new path should exist after rename");

   // POSIX rename() replaces an existing destination; so must this on Windows
   const std::string otherPath = getTempFile();
   require(OSUtils::renameFile(otherPath, newPath), "renameFile should replace an existing destination file");
   requireFalse(OSUtils::pathExists(otherPath), "the source should be gone after replacing the destination");
   require(OSUtils::pathExists(newPath), "the destination should still exist after being replaced");

   requireFalse(OSUtils::renameFile(otherPath, newPath), "renameFile should fail when the source doesn't exist");

   deleteFile(newPath);
}

//******************************************************************************

void TestOSUtils::testSysPlatform() {
   TEST_CASE("testSysPlatform");

#ifdef __linux__
   requireStringEquals("linux", OSUtils::sysPlatform(), "sysPlatform should report 'linux' on this platform");
#elif defined(_WIN32)
   requireStringEquals("windows", OSUtils::sysPlatform(), "sysPlatform should report 'windows' on this platform");
#else
   requireNonEmptyString(OSUtils::sysPlatform(), "sysPlatform should return a non-empty platform name");
#endif
}

//******************************************************************************

void TestOSUtils::testOsName() {
   TEST_CASE("testOsName");

#ifdef _WIN32
   requireStringEquals("unknown", OSUtils::osName(), "osName should report 'unknown' on Windows");
#else
   requireStringEquals("posix", OSUtils::osName(), "osName should report 'posix' on this platform");
#endif
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

   removePath(subDirPath);
   removePath(filePath);
   removePath(dirPath);
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

   removePath(subDirPath);
   removePath(filePath);
   removePath(dirPath);
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
#ifdef _WIN32
   hostName = "untouched";
   requireFalse(OSUtils::getOSHostName(hostName), "getOSHostName is not supported on Windows");
   requireStringEquals("untouched", hostName, "an unsupported getOSHostName should leave its argument alone");
#else
   require(OSUtils::getOSHostName(hostName), "getOSHostName should succeed on this platform");
   requireNonEmptyString(hostName, "host name should be non-empty");
#endif
}

//******************************************************************************

void TestOSUtils::testGetOSUser() {
   TEST_CASE("testGetOSUser");

   std::string user;
#ifdef _WIN32
   user = "untouched";
   requireFalse(OSUtils::getOSUser(user), "getOSUser is not supported on Windows");
   requireStringEquals("untouched", user, "an unsupported getOSUser should leave its argument alone");
#else
   require(OSUtils::getOSUser(user), "getOSUser should succeed on this platform");
   requireNonEmptyString(user, "user name should be non-empty");
#endif
}

//******************************************************************************

void TestOSUtils::testGetOneMinuteLoadAvg() {
   TEST_CASE("testGetOneMinuteLoadAvg");

   require(OSUtils::getOneMinuteLoadAvg() >= 0.0, "one-minute load average should be non-negative");
#ifdef _WIN32
   require(0.0 == OSUtils::getOneMinuteLoadAvg(), "the load averages are not supported on Windows and report 0.0");
   require(0.0 == OSUtils::getFiveMinuteLoadAvg(), "the five-minute load average is not supported on Windows");
   require(0.0 == OSUtils::getFifteenMinuteLoadAvg(), "the fifteen-minute load average is not supported on Windows");
#endif
}

//******************************************************************************

void TestOSUtils::testGetOSName() {
   TEST_CASE("testGetOSName");

   std::string osName;
#ifdef _WIN32
   osName = "untouched";
   requireFalse(OSUtils::getOSName(osName), "getOSName is not supported on Windows");
   requireStringEquals("untouched", osName, "an unsupported getOSName should leave its argument alone");
#else
   require(OSUtils::getOSName(osName), "getOSName should succeed on this platform");
   requireNonEmptyString(osName, "OS name should be non-empty");
#endif
}

//******************************************************************************

void TestOSUtils::testGetOSRelease() {
   TEST_CASE("testGetOSRelease");

   std::string osRelease;
#ifdef _WIN32
   osRelease = "untouched";
   requireFalse(OSUtils::getOSRelease(osRelease), "getOSRelease is not supported on Windows");
   requireStringEquals("untouched", osRelease, "an unsupported getOSRelease should leave its argument alone");
#else
   require(OSUtils::getOSRelease(osRelease), "getOSRelease should succeed on this platform");
   requireNonEmptyString(osRelease, "OS release should be non-empty");
#endif
}

//******************************************************************************

void TestOSUtils::testGetHWCpuCount() {
   TEST_CASE("testGetHWCpuCount");

   int count = 0;
#ifdef __linux__
   require(OSUtils::getHWCpuCount(count), "getHWCpuCount should succeed on Linux");
   require(count > 0, "CPU count should be positive");
#elif defined(_WIN32)
   count = 12345;
   requireFalse(OSUtils::getHWCpuCount(count), "getHWCpuCount is not supported on Windows");
   require(12345 == count, "an unsupported getHWCpuCount should leave its argument alone");
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

void TestOSUtils::testUnsupportedOperations() {
   TEST_CASE("testUnsupportedOperations");

   // not implemented on any platform
   std::string hardwareType;
   requireFalse(OSUtils::getHardwareType(hardwareType), "getHardwareType is not implemented");
   std::string osRevision = "untouched";
   requireFalse(OSUtils::getOSRevision(osRevision), "getOSRevision is not implemented");
   requireStringEquals("untouched", osRevision, "getOSRevision should leave its argument alone");

#ifdef _WIN32
   // the rest are not supported on Windows: each reports "unavailable" with
   // its own documented value, and doesn't touch its output argument
   requireFalse(OSUtils::isUserInGroup("Administrators"), "isUserInGroup is not supported on Windows");

   std::string cpuType = "untouched";
   requireFalse(OSUtils::getHWCpuType(cpuType), "getHWCpuType is not supported on Windows");
   requireStringEquals("untouched", cpuType, "an unsupported getHWCpuType should leave its argument alone");

   require(0 == OSUtils::getHWPhysicalMemoryMB(), "getHWPhysicalMemoryMB is not supported on Windows and reports 0");
   require(0 == OSUtils::getHWCpuSpeedMHz(), "getHWCpuSpeedMHz is not supported on Windows and reports 0");
   require(-1 == OSUtils::getFreeMemoryMB(), "getFreeMemoryMB is not supported on Windows and reports -1");
#endif
}

//******************************************************************************
