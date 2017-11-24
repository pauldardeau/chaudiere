#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "ByteBuffer.h"


namespace chaudiere {

class Utils {
public:
   static void writeFile(const std::string& filePath, const ByteBuffer& blob);
};

}

#endif
