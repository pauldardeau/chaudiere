// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "InvalidKeyException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

InvalidKeyException::InvalidKeyException(const std::string& key) :
   BasicException("Invalid Key: " +  key),
   m_key(key) {
   Logger::logInstanceCreate("InvalidKeyException");
}

//******************************************************************************

InvalidKeyException::InvalidKeyException(const InvalidKeyException& copy) :
   BasicException(copy),
   m_key(copy.m_key) {
   Logger::logInstanceCreate("InvalidKeyException");
}

//******************************************************************************

InvalidKeyException::InvalidKeyException(const InvalidKeyException&& move) :
   BasicException(move),
   m_key(std::move(move.m_key)) {
   Logger::logInstanceCreate("InvalidKeyException");
}

//******************************************************************************

InvalidKeyException::~InvalidKeyException() throw () {
   Logger::logInstanceDestroy("InvalidKeyException");
}

//******************************************************************************

InvalidKeyException& InvalidKeyException::operator=(const InvalidKeyException& copy) {
   if (this == &copy) {
      return *this;
   }
   
   BasicException::operator=(copy);
   m_key = copy.m_key;
   
   return *this;
}

//******************************************************************************

InvalidKeyException& InvalidKeyException::operator=(const InvalidKeyException&& move) {
   BasicException::operator=(move);
   m_key = std::move(move.m_key);
   return *this;
}

//******************************************************************************

const std::string& InvalidKeyException::getKey() const {
   return m_key;
}

//******************************************************************************

