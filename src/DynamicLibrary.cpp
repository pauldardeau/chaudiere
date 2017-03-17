// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <cstdio>
#include <dlfcn.h>

#include "DynamicLibrary.h"
#include "BasicException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

DynamicLibrary::DynamicLibrary() :
   m_hDll(NULL) {
   Logger::logInstanceCreate("DynamicLibrary");
}

//******************************************************************************

DynamicLibrary::DynamicLibrary(const std::string& libraryName) :
   m_hDll(NULL) {
   Logger::logInstanceCreate("DynamicLibrary");

   if (!open(libraryName)) {
      throw BasicException("unable to open library: " + libraryName);
   }
}

//******************************************************************************

DynamicLibrary::~DynamicLibrary() {
   Logger::logInstanceDestroy("DynamicLibrary");
   close();
}

//******************************************************************************

void* DynamicLibrary::resolve(const std::string& functionName) {
   return ::dlsym(m_hDll, functionName.c_str());
}

//******************************************************************************

bool DynamicLibrary::open(const std::string& libraryName) {
   close();
   
   m_hDll = ::dlopen(libraryName.c_str(), RTLD_NOW);
   
   bool rc = (m_hDll != NULL);
   
   if (!rc) {
      Logger::error(std::string("unable to load library '") +
                    libraryName + std::string("'"));
   }
   
   return rc;
}

//******************************************************************************

void DynamicLibrary::close() {
   if (m_hDll == NULL) {
      return;
   }

   ::dlclose(m_hDll);

   m_hDll = NULL;
}

//******************************************************************************

