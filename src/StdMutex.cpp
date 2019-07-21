// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "StdMutex.h"
#include "Logger.h"

static const std::string EMPTY_STRING = "";

using namespace chaudiere;

//******************************************************************************

StdMutex::StdMutex() :
   StdMutex(EMPTY_STRING) {
}

//******************************************************************************

StdMutex::StdMutex(const std::string& mutexName) :
   m_mutexName(mutexName),
   m_isLocked(false) {
   LOG_INSTANCE_CREATE("StdMutex")
}

//******************************************************************************

StdMutex::~StdMutex() {
   LOG_INSTANCE_DESTROY("StdMutex")

   if (m_isLocked) {
      unlock();
   }
}

//******************************************************************************

bool StdMutex::unlock() {
   if (m_isLocked) {
      m_mutex.unlock();
      m_isLocked = false;
      return true;
   } else {
      return false;
   }
}

//******************************************************************************

bool StdMutex::lock() {
   if (!m_isLocked) {
      m_mutex.lock();
      m_isLocked = true;
      return m_isLocked;
   } else {
      return false;
   }
}

//******************************************************************************

bool StdMutex::isLocked() const {
   return m_isLocked;
}

//******************************************************************************

bool StdMutex::haveValidMutex() const {
   return true;
}

//******************************************************************************

const std::string& StdMutex::getName() const {
   return m_mutexName;
}

//******************************************************************************

