#include <stdio.h>
#include "Utils.h"
#include "ByteBuffer.h"

using namespace std;
using namespace chaudiere;


void Utils::WriteFile(const string& filePath, const ByteBuffer& blob) {
   //TODO: implement Utils::WriteFile
}

void Utils::WriteLine(const string& s) {
   printf("%s\n", s.c_str());
}

ByteBuffer* Utils::ReadFile(const std::string& filePath) {
   ByteBuffer* buffer = NULL;
   //TODO: implement Utils::ReadFile
   return buffer;
}

std::string Utils::Md5ForFile(const std::string& filePath) {
   string md5;
   //TODO: implement Utils::Md5ForFile
   return md5;
}

