// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>

#include "OSUtils.h"
#include "StrUtils.h"

static const std::string SLASH = "/";

using namespace chaudiere;

//******************************************************************************

std::string OSUtils::getCurrentDirectory()
{
   std::string currentDirectory;
   
   char* buffCurrentDir = ::getcwd(nullptr, 0);
   
   if (buffCurrentDir != nullptr) {
      currentDirectory = buffCurrentDir;
      free(buffCurrentDir);
   }
   
   return currentDirectory; 
}

//******************************************************************************

std::string OSUtils::pathJoin(const std::string& directory,
                              const std::string& fileName)
{
   std::string joinedPath(directory);
   
   if (!StrUtils::endsWith(directory, SLASH)) {
      joinedPath += SLASH;
   }
   
   joinedPath += fileName;

   return joinedPath;
}

//******************************************************************************

bool OSUtils::pathExists(const std::string& filePath)
{
   FILE* f = fopen(filePath.c_str(), "r");
   if (f != nullptr) {
      fclose(f);
      return true;
   } else {
      return false;
   }
}

//******************************************************************************
