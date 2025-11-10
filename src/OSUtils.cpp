// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <filesystem>
#include <ctime>
#include <cstdint>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/utsname.h>

#ifndef _WIN32
#include <pwd.h>
#include <grp.h>
#endif

#ifdef __linux__
#include <sys/sysinfo.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#elif defined(__FreeBSD__)
#include <sys/sysctl.h>
#elif defined(__sun__)
#include <sys/loadavg.h>
#include <sys/processor.h>
#include <sys/systeminfo.h>
#endif

#include "OSUtils.h"
#include "StrUtils.h"

static const long ONE_MB = 1024 * 1024;


static const unsigned long CRC_TABLE[256] = {
0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,
0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,
0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,
0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,
0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,
0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,
0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,
0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,
0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,
0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,
0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,
0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,
0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,
0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,
0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,
0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,
0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,
0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,
0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,
0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,
0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,
0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,
0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,
0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,
0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,
0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D };

using namespace std;
using namespace std::filesystem;
using namespace chaudiere;

static const std::string DIR_PATH_SEPARATOR = std::string("") + std::filesystem::path::preferred_separator;


//******************************************************************************

string OSUtils::getCurrentDirectory() {
   string currentDirectory;
   char currentDir[2048];
   memset(currentDir, 0, sizeof(currentDir));

   // this call to getcwd will allocate memory buffer
   char* buffCurrentDir = ::getcwd(currentDir, sizeof(currentDir));

   if (buffCurrentDir != nullptr) {
      currentDirectory = buffCurrentDir;
   }

   return currentDirectory;
}

//******************************************************************************

string OSUtils::pathJoin(const string& directory,
                         const string& fileName) {
   string joinedPath(directory);

   if (!StrUtils::endsWith(directory, DIR_PATH_SEPARATOR)) {
      joinedPath += DIR_PATH_SEPARATOR;
   }

   joinedPath += fileName;

   return joinedPath;
}

//******************************************************************************

bool OSUtils::directoryExists(const string& directory) {
   return is_directory(path{directory});
}

//******************************************************************************

bool OSUtils::pathExists(const string& filePath) {
   return exists(path{filePath});
}

//******************************************************************************

bool OSUtils::createDirectory(const string& directory) {
   const mode_t mode = S_IRWXU |  // user read/write/execute
                       S_IRGRP |  // group read
                       S_IXGRP |  // group execute
                       S_IROTH |  // other read
                       S_IXOTH;   // other execute
   const int rc = ::mkdir(directory.c_str(), mode);
   return (0 == rc);
}

//******************************************************************************

bool OSUtils::createPrivateDirectory(const string& directory) {
   const int rc = ::mkdir(directory.c_str(), S_IRWXU);
   return (0 == rc);
}

//******************************************************************************

void OSUtils::programExit(int exitCode) {
   exit(exitCode);
}

//******************************************************************************

long OSUtils::getFileSize(const string& filePath) {
   long fileSize = -1L;

   path aFilePath{filePath};
   if (exists(aFilePath)) {
      fileSize = file_size(aFilePath);
   }

   return fileSize;
}

//******************************************************************************

bool OSUtils::deleteFile(const string& filePath) {
   return (0 == unlink(filePath.c_str()));
}

//******************************************************************************

bool OSUtils::renameFile(const string& oldFilePath,
                         const string& newFilePath) {
   return (0 == rename(oldFilePath.c_str(), newFilePath.c_str()));
}

//******************************************************************************

string OSUtils::sysPlatform() {
   string name;

#ifdef __linux__
   name = "linux";
#elif defined(__APPLE__)
   name = "mac";  // darwin, osx
#elif defined(__FreeBSD__)
   name = "freebsd";
#elif defined(__NetBSD__)
   name = "netbsd";
#elif defined(__OpenBSD__)
   name = "openbsd";
#elif defined(_WIN32) || defined(WIN32)
   name = "windows";
#elif defined(__sun__)
   name = "solaris";
#else
   name = "unknown";
#endif

   return name;
}

//******************************************************************************

string OSUtils::osName() {
#if defined(_POSIX_VERSION)
   return string("posix");
#else
   return string("unknown");
#endif
}

//******************************************************************************

void OSUtils::splitExt(const string& filePath,
                       vector<string>& pathParts) {
   const string::size_type posLastDot = filePath.find_last_of(".");
   if (posLastDot != string::npos) {
      pathParts.push_back(filePath.substr(0, posLastDot));  // root
      pathParts.push_back(filePath.substr(posLastDot, string::npos));  // ext
   } else {
      pathParts.push_back(filePath);
      pathParts.push_back(string(""));
   }
}

//******************************************************************************

long OSUtils::currentTimeMillis() {
   //TODO: implement OSUtils::currentTimeMillis
   return 0L;
}

//******************************************************************************

vector<string> OSUtils::listFilesInDirectory(const string& dirPath) {
   vector<string> listFiles;

   const path aDirPath{dirPath};

   for (const auto& entry : directory_iterator{aDirPath}) {
      if (!entry.is_directory()) {
         listFiles.push_back(entry.path().filename().string());
      }
   }

   return listFiles;
}

//******************************************************************************

vector<string> OSUtils::listDirsInDirectory(const string& dirPath) {
   vector<string> listSubdirs;

   const path aDirPath{dirPath};

   for (const auto& entry : directory_iterator{aDirPath}) {
      if (entry.is_directory()) {
         listSubdirs.push_back(entry.path().filename().string());
      }
   }

   return listSubdirs;
}

//******************************************************************************

unsigned long OSUtils::crc32ForBuffer(unsigned long inCrc32,
                                      const void *buf,
                                      size_t bufLen) {
   /** accumulate crc32 for buffer **/
   unsigned long crc32 = inCrc32 ^ 0xFFFFFFFF;
   unsigned char* byteBuf = (unsigned char*) buf;

   for (size_t i = 0; i < bufLen; ++i) {
      crc32 = (crc32 >> 8) ^ CRC_TABLE[(crc32 ^ byteBuf[i]) & 0xFF];
   }

   return (crc32 ^ 0xFFFFFFFF);
}

//******************************************************************************

bool OSUtils::crc32ForFile(const string& filePath, string& crc32) {
   FILE* f = ::fopen(filePath.c_str(), "r");
   if (f != nullptr) {
      unsigned char buf[8192];
      size_t bufLen;

      unsigned long crc32Value = 0L;

      while (1) {
         bufLen = ::fread(buf, 1, 8192, f);
         if (bufLen == 0) {
            if (ferror(f)) {
               // error reading file
               fclose(f);
               return false;
            }
            break;
         } else {
            crc32Value = crc32ForBuffer(crc32Value, buf, bufLen);
         }
      }

      fclose(f);

      // convert crc32 numeric value to string
      char crcValueAsString[20];
      snprintf(crcValueAsString, 20, "%lx", crc32Value);
      crc32 = crcValueAsString;

      return true;
   } else {
      // unable to open file
      return false;
   }
}

//******************************************************************************

bool OSUtils::isUserInGroup(const string& groupName)
{
#ifdef __unix__
   const gid_t userPrimaryGroup = getgid();

   if (userPrimaryGroup > 0) {
      const group* pGroup = getgrnam(groupName.c_str());

      if (pGroup != nullptr) {
         if (pGroup->gr_gid == userPrimaryGroup) {
            // yes, user is in group (it's their primary group)
            return true;
         } else {
            // check secondary groups
#if defined(__sun__)
            const int nMaxGroups = NGROUPS_UMAX;
            gid_t mygidset[NGROUPS_UMAX];
#else
            const int nMaxGroups = NGROUPS_MAX;
            gid_t mygidset[NGROUPS_MAX];
#endif
            const int nGroups = getgroups(nMaxGroups, mygidset);

            if (nGroups > -1) {
               for (int i = 0; i < nGroups; ++i) {
                  if (mygidset[i] == pGroup->gr_gid) {
                     // yes, user is in group (it's a secondary group)
                     return true;
                  }
               }
            }
         }
      } else {
         // this group doesn't exist
      }
   }
#endif
   return false;
}

//******************************************************************************

bool OSUtils::getHWCpuCount(int& count)
{
   bool rc = false;

#if defined(_WIN32) || defined(WIN32)
   char szRegistryKey[256];
   HKEY keyCPU;

   for (int cpuIndex = 0; ; cpuIndex++) {
      snprintf(szRegistryKey,
               255,
               "Hardware\\Description\\System\\CentralProcessor\\%d",
               cpuIndex);

      if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                          szRegistryKey,
                                          0,
                                          KEY_READ,
                                          &keyCPU)) {
         count = cpuIndex;
         break;
      }
   }

   rc = true;

#elif defined(__sun__)
   count = sysconf(_SC_NPROCESSORS_ONLN);
   rc = true;
#elif defined(__linux__)
   count = get_nprocs_conf();
   rc = true;
#elif defined(__APPLE__) || defined(__FreeBSD__)
   int cpuCount = 0;
   size_t size = sizeof(cpuCount);
   if (0 == sysctlbyname("hw.ncpu", &cpuCount, &size, nullptr, 0)) {
      count = cpuCount;
      rc = true;
   }
#endif

   return rc;
}

//******************************************************************************

bool OSUtils::getHWCpuType(string& cpuType)
{
   bool rc = false;

#if defined(_WIN32) || defined(WIN32)
   HKEY keyCPU;

   if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       "Hardware\\Description\\System\\CentralProcessor\\0",
                                       0,
                                       KEY_READ,
                                       &keyCPU)) {
      if (readRegistryValue(keyCPU, "ProcessorNameString\0", cpuType)) {
         rc = true;
      }
      ::RegCloseKey(keyCPU);
   }
#elif defined(__sun__)
   processor_info_t pi;
   if (0 == processor_info(0, &pi)) {
      cpuType = pi.pi_processor_type;
      rc = true;
   }
#elif defined(__FreeBSD__)
   char szCpuType[128];
   size_t size = 127;
   if (0 == sysctlbyname("hw.model", &szCpuType, &size, nullptr, 0)) {
      cpuType = szCpuType;
      rc = true;
   }
#elif defined(__APPLE__)
   int cpuTypeCode = 0;
   size_t size = sizeof(cpuTypeCode);
   if (0 == sysctlbyname("hw.cputype", &cpuTypeCode, &size, nullptr, 0)) {
      int cpuSubType = 0;
      if (0 == sysctlbyname("hw.cpusubtype", &cpuSubType, &size, nullptr, 0)) {
         //TODO: handle hw.cpusubtype for Apple
      }
   }
#endif

   return rc;
}

//******************************************************************************

bool OSUtils::getHardwareType(string& hardwareType)
{
   bool rc = false;
   hardwareType = "** unknown **";

#ifdef __sun__
   char szPlatform[128];
   if (sysinfo(SI_PLATFORM, szPlatform, 127) > -1) {
      hardwareType = szPlatform;
      rc = true;
   }
#endif

   return rc;
}

//******************************************************************************

int OSUtils::getHWPhysicalMemoryMB()
{
   int physicalMemoryMB = 0;

#if defined(_WIN32) || defined(WIN32)
   MEMORYSTATUS memStat;
   memset(&memStat, 0, sizeof(MEMORYSTATUS));

   ::GlobalMemoryStatus(&memStat);
   const SIZE_T dwTotalBytes = memStat.dwTotalPhys;
   physicalMemoryMB = dwTotalBytes / ONE_MB;
#elif defined(__sun__)
   const long numPages = sysconf(_SC_PHYS_PAGES);
   const long pageSize = sysconf(_SC_PAGE_SIZE);
   const longlong_t mem = (longlong_t) ((longlong_t) numPages * (longlong_t) pageSize);
   physicalMemoryMB = mem / ONE_MB;
#elif defined(__linux__)
   const long int numPages = get_phys_pages();
   const int pageSize = getpagesize();
   uint64_t mem = (uint64_t) ((uint64_t) numPages * (uint64_t) pageSize);
   physicalMemoryMB = mem / ONE_MB;
#elif defined(__APPLE__)
   uint64_t physMemory = 0;
   size_t size = sizeof(physMemory);
   if (0 == sysctlbyname("hw.memsize", &physMemory, &size, nullptr, 0)) {
      physicalMemoryMB = physMemory / ONE_MB;
   }
#elif defined(__FreeBSD__)
   uint64_t physMemory = 0;
   size_t size = sizeof(physMemory);
   if (0 == sysctlbyname("vm.kmem_size_max", &physMemory, &size, nullptr, 0)) {
      physicalMemoryMB = physMemory / ONE_MB;
   }
#endif

   return physicalMemoryMB;
}

//******************************************************************************

int OSUtils::getHWCpuSpeedMHz()
{
   int cpuSpeedMHz = 0;

#if defined(_WIN32) || defined(WIN32)
   HKEY keyCPU;

   const auto rc = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                  "Hardware\\Description\\System\\CentralProcessor\\0",
                                  0,
                                  KEY_READ,
                                  &keyCPU);
   if (ERROR_SUCCESS == rc) {
      DWORD dwCpuSpeed = 0;
      DWORD dwSize = sizeof(DWORD);

      if (ERROR_SUCCESS == ::RegQueryValueEx(keyCPU,
                                             "~MHz\0",
                                             nullptr,
                                             nullptr,
                                             (LPBYTE) &dwCpuSpeed,
                                             &dwSize)) {
         cpuSpeedMHz = dwCpuSpeed;
      }

      ::RegCloseKey(keyCPU);
   }
#elif defined(__sun__)
   processor_info_t pi;
   if (0 == processor_info(0, &pi)) {
      cpuSpeedMHz = pi.pi_clock;
   }
#elif defined(__FreeBSD__)
   int cpuSpeed = 0;
   size_t size = sizeof(cpuSpeed);
   if (0 == sysctlbyname("hw.clockrate", &cpuSpeed, &size, nullptr, 0)) {
      cpuSpeedMHz = cpuSpeed;
   }
#elif defined(__APPLE__)
   uint64_t cpuSpeed = 0;
   size_t size = sizeof(cpuSpeed);
   if (0 == sysctlbyname("hw.cpufrequency", &cpuSpeed, &size, nullptr, 0)) {
      cpuSpeedMHz = cpuSpeed / 1000000;
   }
#elif defined(__linux__)
   //TODO: is there a reliable way to get cpu clock speed on linux?
#endif

   return cpuSpeedMHz;
}

//******************************************************************************

bool OSUtils::getOSHostName(string& hostName)
{
   bool rc = false;

#if defined(_WIN32) || defined(WIN32)
   char computerName[255];
   unsigned long nChars = 254;
   memset(computerName, 0, sizeof(computerName));

   if (::GetComputerName(computerName, &nChars)) {
      hostName = computerName;
      rc = true;
   }
#elif defined(__unix__)
   struct utsname data;
   if (uname(&data) > -1) {
      hostName = data.nodename;
      rc = true;
   }
#endif

   return rc;
}

//******************************************************************************

bool OSUtils::getOSUser(string& user)
{
   bool rc = false;

#if defined(_WIN32) || defined(WIN32)
   unsigned long nUserChars = 254;
   char userName[255];
   memset(userName, 0, sizeof(userName));

   if (::GetUserName(userName, &nUserChars)) {
      user = userName;
      rc = true;
   }
#elif defined(__unix__)
   struct passwd* passwordData = getpwuid(getuid());
   if ((passwordData != nullptr) &&
       (passwordData->pw_name != nullptr)) {
      user = passwordData->pw_name;
      rc = true;
   }
#endif

   return rc;
}

//******************************************************************************

double OSUtils::getOneMinuteLoadAvg()
{
#ifdef __unix__
   double load_avg[3];
   if (getloadavg(load_avg, 3) > -1) {
      return load_avg[0];
   }
#endif
   return 0.0;
}

//******************************************************************************

double OSUtils::getFiveMinuteLoadAvg()
{
#ifdef __unix__
   double load_avg[3];
   if (getloadavg(load_avg, 3) > -1) {
      return load_avg[1];
   }
#endif
   return 0.0;
}

//******************************************************************************

double OSUtils::getFifteenMinuteLoadAvg()
{
#ifdef __unix__
   double load_avg[3];
   if (getloadavg(load_avg, 3) > -1) {
      return load_avg[2];
   }
#endif
   return 0.0;
}

//******************************************************************************

bool OSUtils::getOSName(string& osName)
{
   bool rc = false;

#ifdef __unix__
   struct utsname data;
   if (uname(&data) > -1) {
      osName = data.sysname;
      rc = true;
   }
#endif

   return rc;
}

//******************************************************************************

bool OSUtils::getOSRelease(string& osRelease)
{
   bool rc = false;

#ifdef __unix__
   struct utsname data;
   if (uname(&data) > -1) {
      osRelease = data.release;
      rc = true;
   }
#endif

   return rc;
}

//******************************************************************************

bool OSUtils::getOSRevision(string& osRevision)
{
   bool rc = false;

#if defined(_WIN32) || defined(WIN32)
   OSVERSIONINFO osVersionInfo;
   osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   if (::GetVersionEx(&osVersionInfo)) {
      rc = true;
      osRevision = osVersionInfo.szCSDVersion;
   }
#elif defined(__sun__)
   struct utsname data;
   if (uname(&data) > -1) {
      osRevision = data.version;
      rc = true;
   }
#endif

   return rc;
}

//******************************************************************************

int OSUtils::getFreeMemoryMB()
{
   int freeMemoryMB = -1;

#ifdef __sun__
   long freePages = sysconf(_SC_AVPHYS_PAGES);
   long pageSize = sysconf(_SC_PAGE_SIZE);
   longlong_t freeMem = (longlong_t) freePages * (longlong_t) pageSize;
   freeMemoryMB = freeMem / ONE_MB;
#elif defined(__linux__)
   uint64_t freePages = get_avphys_pages();
   const int pageSize = getpagesize();
   uint64_t mem = (uint64_t) ((uint64_t) freePages * (uint64_t) pageSize);
   freeMemoryMB = mem / ONE_MB;
#elif defined(__FreeBSD__)
   uint64_t freePages = 0;
   size_t size = sizeof(freePages);
   if (0 == sysctlbyname("vm.stats.vm.v_free_count", &freePages, &size, nullptr, 0)) {
      int pageSize = 0;
      size = sizeof(pageSize);
      if (0 == sysctlbyname("hw.pagesize", &pageSize, &size, nullptr, 0)) {
         freeMemoryMB = ((freePages * pageSize) / ONE_MB);
      }
   }
#endif

   return freeMemoryMB;
}

//******************************************************************************

bool OSUtils::getOSCurrentTimestamp(string& timestamp)
{
   bool rc = false;

   time_t current_time;
   time(&current_time);

   struct tm* pTM = localtime(&current_time);

   if (pTM != nullptr) {
      char buffer[128];
      buffer[0] = '\0';

      snprintf(buffer,
               127,
               "%04d-%02d-%02d %02d:%02d:%02d",
               pTM->tm_year + 1900, // year
               pTM->tm_mon + 1,     //month
               pTM->tm_mday,        // day of month
               pTM->tm_hour,        // hour
               pTM->tm_min,         // minutes
               pTM->tm_sec);        // seconds

      rc = true;
      timestamp = buffer;
   }

   return rc;
}

//******************************************************************************

