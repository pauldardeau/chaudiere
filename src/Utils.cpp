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

ByteBuffer* Utils::ReadFile(const std::string& filePath) {
   ByteBuffer* buffer = nullptr;
   FILE* f = fopen(filePath.c_str(), "rb");
   if (f != nullptr) {
      fseek(f, 0, SEEK_END);
      const long fileSize = ftell(f);
      fseek(f, 0, SEEK_SET);
      if (fileSize >= 0) {
         const size_t fileBytes = (size_t) fileSize;
         buffer = new ByteBuffer(fileBytes);
         // size/nmemb swapped intentionally vs. a "read 1 object of
         // fileBytes size" call: this way the return value is a byte
         // count, comparable directly against fileBytes below.
         size_t bytesRead = fread(buffer->data(), 1, fileBytes, f);
         if (bytesRead < fileBytes) {
            delete buffer;
            buffer = nullptr;
         }
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

