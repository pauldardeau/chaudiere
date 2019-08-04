#include <stdio.h>
#include "Utils.h"
#include "ByteBuffer.h"

using namespace std;
using namespace chaudiere;


bool Utils::WriteFile(const string& filePath, const ByteBuffer& buffer) {
   bool success = false;

   FILE* f = fopen(filePath.c_str(), "wb");
   if (f != nullptr) {
      size_t bytesWritten = 0;
      if (buffer.size() > 0) {
         bytesWritten = fwrite(buffer.const_data(), 1, buffer.size(), f);
         fflush(f);
      }
      fclose(f);
      success = (bytesWritten == buffer.size());
   }

   return success;
}

void Utils::Write(const string& s) {
   printf("%s", s.c_str());
}

void Utils::WriteLine(const string& s) {
   printf("%s\n", s.c_str());
}

std::unique_ptr<ByteBuffer> Utils::ReadFile(const std::string& filePath) {
   std::unique_ptr<ByteBuffer> buffer;
   FILE* f = fopen(filePath.c_str(), "rb");
   if (f != nullptr) {
      fseek(f, 0, SEEK_END);
      unsigned long fileBytes = ftell(f);
      fseek(f, 0, SEEK_SET);
      buffer = std::unique_ptr<ByteBuffer>(new ByteBuffer(fileBytes));
      size_t bytesRead = fread(buffer->data(), fileBytes, 1, f);
      if (bytesRead < fileBytes) {
         buffer.reset();
         buffer = nullptr;
      }
      fclose(f);
   }
   return buffer;
}

std::string Utils::Md5ForFile(const std::string& filePath) {
   string md5;
   //TODO: implement Utils::Md5ForFile
   return md5;
}

