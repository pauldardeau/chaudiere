// A consumer of the installed chaudiere headers and library: exercises the
// APIs that are supported on every platform, including Windows. Built and run
// by run.cmake against an install prefix, using nothing from the source tree.

#include <cstdio>
#include <exception>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "DateTime.h"
#include "Logger.h"
#include "NumberFormatException.h"
#include "OSUtils.h"
#include "StrUtils.h"

using namespace chaudiere;

namespace {

int g_failures = 0;

void check(bool ok, const char* what) {
   if (!ok) {
      ++g_failures;
      std::printf("FAILED: %s\n", what);
   }
}

// records what is logged, so the Logger facade can be observed
class RecordingLogger : public Logger {
public:
   LogLevel getLogLevel() const override { return m_level; }
   void setLogLevel(LogLevel logLevel) override { m_level = logLevel; }
   bool isLoggingInstanceLifecycles() const override { return false; }
   void setLogInstanceLifecycles(bool) override {}
   void logInstanceCreate(const std::string&) override {}
   void logInstanceDestroy(const std::string&) override {}
   void logMessage(LogLevel logLevel, const std::string& logMessage) override {
      m_messages.push_back(std::to_string(static_cast<int>(logLevel)) + ":" + logMessage);
   }
   bool isLoggingLevel(LogLevel logLevel) const override { return logLevel <= m_level; }
   void logOccurrence(const std::string&, const std::string&) override {}

   std::vector<std::string> m_messages;
private:
   LogLevel m_level = Info;
};

void checkStrUtils() {
   check(42 == StrUtils::parseInt(std::string("42")), "parseInt");
   check("42" == StrUtils::toString(42), "toString(int)");
   check(StrUtils::startsWith(std::string("chaudiere"), std::string("chau")), "startsWith");

   // the header seen here must be the C++20 one the library was built with
   check(__cplusplus >= 202002L, "__cplusplus reports C++20");
   check(7 == StrUtils::parseInt(std::string_view("7")), "string_view overload");

   bool caught = false;
   try {
      StrUtils::parseInt(std::string("not a number"));
   } catch (const NumberFormatException&) {
      caught = true;
   }
   check(caught, "parseInt throws NumberFormatException");

   // thrown as a chaudiere exception, catchable as the poivre base and as std::exception
   caught = false;
   try {
      StrUtils::parseInt(std::string("not a number"));
   } catch (const std::exception&) {
      caught = true;
   }
   check(caught, "NumberFormatException is a std::exception");

   // out of range is an error, never a clamped value, on either data model
   if (sizeof(long) == 4) {
      caught = false;
      try {
         StrUtils::parseLong(std::string("4294967296"));
      } catch (const NumberFormatException&) {
         caught = true;
      }
      check(caught, "parseLong(2^32) throws where long is 32 bits");
   } else {
      check(4294967296L == StrUtils::parseLong(std::string("4294967296")), "parseLong(2^32) where long is 64 bits");
   }
}

void checkDateTime() {
   DateTime now;
   check(now.timeIntervalSince1970() > 1.7e9, "DateTime reflects the current time");

   DateTime date(0);
   check(DateTime::populateFromUnixTime(date, 1700000000.0), "populateFromUnixTime");
   check(date.getYear() >= 2023, "populated year");
}

void checkOSUtils() {
   const std::string base = OSUtils::getCurrentDirectory();
   check(!base.empty(), "getCurrentDirectory");
   check(OSUtils::directoryExists(base), "directoryExists(current directory)");

   const std::string dir = OSUtils::pathJoin(base, "chaudiere_consumer_dir");
   std::error_code ec;
   std::filesystem::remove_all(dir, ec);

   check(!OSUtils::directoryExists(dir), "directory absent before creation");
   check(OSUtils::createDirectory(dir), "createDirectory");
   check(OSUtils::directoryExists(dir), "directory present after creation");
   check(!OSUtils::createDirectory(dir), "createDirectory fails when it exists");

   const std::string privateDir = OSUtils::pathJoin(base, "chaudiere_consumer_private_dir");
   std::filesystem::remove_all(privateDir, ec);
#ifdef _WIN32
   // not supported there: no directory is created and none is claimed
   check(!OSUtils::createPrivateDirectory(privateDir), "createPrivateDirectory is unsupported on Windows");
   check(!OSUtils::directoryExists(privateDir), "createPrivateDirectory created nothing on Windows");
#else
   check(OSUtils::createPrivateDirectory(privateDir), "createPrivateDirectory");
   check(OSUtils::directoryExists(privateDir), "private directory present");
#endif

   std::filesystem::remove_all(dir, ec);
   std::filesystem::remove_all(privateDir, ec);
}

void checkLogger() {
   RecordingLogger* recorder = new RecordingLogger;
   Logger::setLogger(recorder);   // the facade now owns it

   check(Logger::getLogger() == recorder, "getLogger returns what setLogger installed");
   Logger::info("hello");
   check(recorder->m_messages.size() == 1 && recorder->m_messages[0] == std::to_string(static_cast<int>(Info)) + ":hello",
         "Logger::info reaches the installed logger");
   check(Logger::isLogging(Info) && !Logger::isLogging(Debug), "isLogging follows the logger's level");

   Logger::shutdown();
   check(nullptr == Logger::getLogger(), "shutdown removes the logger");
}

}

int main() {
   checkStrUtils();
   checkDateTime();
   checkOSUtils();
   checkLogger();

   if (0 == g_failures) {
      std::printf("CONSUMER-OK\n");
      return 0;
   }
   std::printf("CONSUMER-FAILED: %d check(s)\n", g_failures);
   return 1;
}
