// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "NumberFormatException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

NumberFormatException::NumberFormatException() :
   BasicException("") {
   LOG_INSTANCE_CREATE("NumberFormatException")
}

//******************************************************************************

NumberFormatException::NumberFormatException(const std::string& s) :
   BasicException(s) {
   LOG_INSTANCE_CREATE("NumberFormatException")
}

//******************************************************************************

NumberFormatException::NumberFormatException(const NumberFormatException& copy) :
   BasicException(copy) {
   LOG_INSTANCE_CREATE("NumberFormatException")
}

//******************************************************************************

NumberFormatException::~NumberFormatException() throw () {
   LOG_INSTANCE_DESTROY("NumberFormatException")
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

