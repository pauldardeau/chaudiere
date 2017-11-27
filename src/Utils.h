#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "ByteBuffer.h"

namespace chaudiere {

class Utils {
public:
   static bool WriteFile(const std::string& filePath, const ByteBuffer& blob);
   static void Write(const std::string& s);
   static void WriteLine(const std::string& s);
   static ByteBuffer* ReadFile(const std::string& filePath);
   static std::string Md5ForFile(const std::string& filePath);
};

}

#endif
