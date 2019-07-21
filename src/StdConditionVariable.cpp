// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "StdConditionVariable.h"
#include "StdMutex.h"
#include "BasicException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

StdConditionVariable::StdConditionVariable() {
   LOG_INSTANCE_CREATE("StdConditionVariable")
}

//******************************************************************************

StdConditionVariable::StdConditionVariable(const std::string& name) :
   m_name(name) {
   LOG_INSTANCE_CREATE("StdConditionVariable")
}

//******************************************************************************

StdConditionVariable::~StdConditionVariable() {
   LOG_INSTANCE_DESTROY("StdConditionVariable")
}

//******************************************************************************

bool StdConditionVariable::wait(Mutex* mutex) {
   if (mutex) {
      StdMutex* stdMutex =
         dynamic_cast<StdMutex*>(mutex);
      
      if (stdMutex) {
         std::unique_lock<std::mutex> lock(stdMutex->getPlatformPrimitive());
         m_cond.wait(lock);
         return true;
      } else {
         LOG_ERROR("mutex must be an instance of StdMutex")
      }
   } else {
      LOG_ERROR("no mutex given to wait on")
   }
   
   return false;
}

//******************************************************************************

void StdConditionVariable::notifyOne() {
   m_cond.notify_one();
}

//******************************************************************************

void StdConditionVariable::notifyAll() {
   m_cond.notify_all();
}

//******************************************************************************

const std::string& StdConditionVariable::getName() const {
   return m_name;
}

//******************************************************************************
