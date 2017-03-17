// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "NumberFormatException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

NumberFormatException::NumberFormatException() :
   BasicException("") {
   Logger::logInstanceCreate("NumberFormatException");
}

//******************************************************************************

NumberFormatException::NumberFormatException(const std::string& s) :
   BasicException(s) {
   Logger::logInstanceCreate("NumberFormatException");
}

//******************************************************************************

NumberFormatException::NumberFormatException(const NumberFormatException& copy) :
   BasicException(copy) {
   Logger::logInstanceCreate("NumberFormatException");
}

//******************************************************************************

NumberFormatException::~NumberFormatException() throw () {
   Logger::logInstanceDestroy("NumberFormatException");
}

//******************************************************************************

NumberFormatException& NumberFormatException::operator=(const NumberFormatException& copy) {
   if (this == &copy) {
      return *this;
   }
   
   BasicException::operator=(copy);
   
   return *this;
}

//******************************************************************************

