// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_OSUTILS_H
#define CHAUDIERE_OSUTILS_H

#include <string>
#include <vector>


namespace chaudiere
{

   /**
    * OSUtils is a collection of OS related utilities that are frequently used
    *
    * Windows (MSVC) support is deliberately limited. Everything declared here
    * links on Windows, but the functions fall into three groups there.
    *
    * 1. Implemented on Windows: getCurrentDirectory, directoryExists,
    *    pathExists, pathJoin, createDirectory, getFileSize, deleteFile,
    *    renameFile, sysPlatform (returns "windows"), splitExt,
    *    listFilesInDirectory, listDirsInDirectory, crc32ForBuffer,
    *    crc32ForFile, programExit and getOSCurrentTimestamp.
    *    - createPrivateDirectory is the exception among the directory
    *      functions: it returns false and creates nothing (a private directory
    *      needs an ACL, which is not implemented).
    *    - getFileSize returns -1 for a file whose size does not fit in a long
    *      (2 GiB or more where long is 32 bits, as on Windows). That check
    *      applies on every platform.
    *    - Narrow (std::string) paths are interpreted in the process's active
    *      ANSI code page on Windows. Names that code page cannot represent
    *      are not supported, and there are no UTF-8 or wide-character
    *      overloads.
    *
    * 2. Not supported on Windows (no implementation; they report
    *    "unavailable" and leave their output arguments untouched):
    *    isUserInGroup, getHWCpuCount, getHWCpuType, getOSHostName, getOSUser,
    *    getOSName and getOSRelease return false; getHWPhysicalMemoryMB returns
    *    0; getFreeMemoryMB returns -1; the load averages return 0.0. Callers
    *    must check the return value, as none of these reports success.
    *    osName returns "unknown" there (it returns "posix" where POSIX is
    *    available).
    *
    * 3. Existing stubs on every platform, Windows included (not a Windows
    *    limitation; unchanged by the Windows port):
    *    - currentTimeMillis is NOT implemented: it always returns 0.
    *    - getOSRevision always returns false and leaves its argument alone.
    *    - getHardwareType always returns false, but does set its argument to
    *      "** unknown **".
    *    - getHWCpuSpeedMHz returns 0 on Linux and Windows (it is implemented
    *      only on FreeBSD and macOS).
    */
   class OSUtils
   {
   public:
      /**
       * Retrieves the current directory
       * @return the current directory
       */
      static std::string getCurrentDirectory();

      /**
       * Determines if the specified directory exists
       * @param directory the directory whose existence is to be tested
       * @return boolean indicating if the directory exists
       */
      static bool directoryExists(const std::string& directory);

      /**
       * Joins (concatenates) a directory path and file name and will include the path delimiter if necessary
       * @param directory the path for the directory
       * @param fileName the name of the file
       * @return joined directory path and file name
       */
      static std::string pathJoin(const std::string& directory,
                                  const std::string& fileName);

      /**
       * Determines if the specified file path exists
       * @param filePath the file path whose existence is to be tested
       * @return boolean indicating if the file path exists
       */
      static bool pathExists(const std::string& filePath);

      /**
       * Creates a new directory of the specified name
       * @param directory name of new directory to create
       * @return boolean indicating if the directory was created
       */
      static bool createDirectory(const std::string& directory);

     /**
      * Creates a new private directory of the specified name
      * @param directory name of new directory to create
      * @return boolean indicating if the directory was created
      */
     static bool createPrivateDirectory(const std::string& directory);

     /**
      *
      * @param
      */
     static void programExit(int exitCode);

     /**
      *
      * @param
      * @return
      */
     static long getFileSize(const std::string& filePath);

     /**
      *
      * @param
      * @return
      */
     static bool deleteFile(const std::string& filePath);

     /**
      *
      * @param
      * @param
      * @return
      */
     static bool renameFile(const std::string& oldFilePath, const std::string& newFilePath);

     /**
      *
      * @return
      */
     static std::string sysPlatform();

     /**
      *
      * @return
      */
     static std::string osName();

     /**
      *
      * @param filePath
      * @return
      */
     static void splitExt(const std::string& filePath,
                          std::vector<std::string>& pathParts);

     /**
      *
      * @return
      */
     static long currentTimeMillis();

     /**
      *
      * @param dirPath
      * @return
      */
     static std::vector<std::string> listFilesInDirectory(const std::string& dirPath);

     /**
      *
      * @param dirPath
      * @return
      */
     static std::vector<std::string> listDirsInDirectory(const std::string& dirPath);

      /**
       *
       * @param inCrc32
       * @param buf
       * @param bufLen
       * @return
       */
      static unsigned long crc32ForBuffer(unsigned long inCrc32,
                                          const void *buf,
                                          size_t bufLen);

      /**
       * Computes the CRC-32 value for the specified file
       * @param filePath the path to the file
       * @param crc32 will be populated with the CRC-32 value as a hex string
       * @return boolean indicating if the CRC value was obtained for the file
       */
      static bool crc32ForFile(const std::string& filePath,
                               std::string& crc32);

      static bool isUserInGroup(const std::string& groupName);

      static bool getHWCpuCount(int& count);

      static bool getHWCpuType(std::string& cpuType);

      static bool getHardwareType(std::string& hardwareType);

      static int getHWPhysicalMemoryMB();

      static int getHWCpuSpeedMHz();

      static bool getOSHostName(std::string& hostName);

      static bool getOSUser(std::string& user);

      static double getOneMinuteLoadAvg();
      static double getFiveMinuteLoadAvg();
      static double getFifteenMinuteLoadAvg();

      static bool getOSName(std::string& osName);
      static bool getOSRelease(std::string& osName);
      static bool getOSRevision(std::string& osRevision);

      static int getFreeMemoryMB();

      static bool getOSCurrentTimestamp(std::string& timestamp);
   };

}

#endif
