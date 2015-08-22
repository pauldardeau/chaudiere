// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "NumberFormatException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

NumberFormatException::NumberFormatException() noexcept :
   BasicException("") {
   Logger::logInstanceCreate("NumberFormatException");
}

//******************************************************************************

NumberFormatException::NumberFormatException(const std::string& s) noexcept :
   BasicException(s) {
   Logger::logInstanceCreate("NumberFormatException");
}

//******************************************************************************

NumberFormatException::NumberFormatException(const NumberFormatException& copy) noexcept :
   BasicException(copy) {
   Logger::logInstanceCreate("NumberFormatException");
}

//******************************************************************************

NumberFormatException::~NumberFormatException() noexcept {
   Logger::logInstanceDestroy("NumberFormatException");
}

//******************************************************************************

NumberFormatException& NumberFormatException::operator=(const NumberFormatException& copy) noexcept {
   if (this == &copy) {
      return *this;
   }
   
   BasicException::operator=(copy);
   
   return *this;
}

//******************************************************************************

