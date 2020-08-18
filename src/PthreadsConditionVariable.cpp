// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "PthreadsConditionVariable.h"
#include "PthreadsMutex.h"
#include "BasicException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

PthreadsConditionVariable::PthreadsConditionVariable() :
   m_initialized(false) {
   LOG_INSTANCE_CREATE("PthreadsConditionVariable")

   if (0 != ::pthread_cond_init(&m_cond, nullptr)) {
      LOG_ERROR("unable to create condition variable (pthreads)")
      throw BasicException("unable to create condition variable (pthreads)");
   } else {
      m_initialized = true;
   }
}

//******************************************************************************

PthreadsConditionVariable::PthreadsConditionVariable(const std::string& name) :
   m_initialized(false),
   m_name(name) {
   LOG_INSTANCE_CREATE("PthreadsConditionVariable")

   if (0 != ::pthread_cond_init(&m_cond, nullptr)) {
      LOG_ERROR("unable to create condition variable (pthreads)")
      throw BasicException("unable to create condition variable (pthreads)");
   } else {
      m_initialized = true;
   }
}

//******************************************************************************

PthreadsConditionVariable::~PthreadsConditionVariable() {
   LOG_INSTANCE_DESTROY("PthreadsConditionVariable")
   if (m_initialized) {
      ::pthread_cond_destroy(&m_cond);
   }
}

//******************************************************************************

bool PthreadsConditionVariable::wait(Mutex* mutex) {
   if (m_initialized) {
      if (mutex) {
         PthreadsMutex* pthreadsMutex =
            dynamic_cast<PthreadsMutex*>(mutex);
         
         if (pthreadsMutex) {
            if (0 != ::pthread_cond_wait(&m_cond,
                                         &pthreadsMutex->getPlatformPrimitive())) {
               LOG_ERROR("unable to wait on condition variable")
            } else {
               return true;
            }
         } else {
            LOG_ERROR("mutex must be an instance of PthreadsMutex")
         }
      } else {
         LOG_ERROR("no mutex given to wait on")
      }
   } else {
      LOG_ERROR("unable to wait on condition variable that hasn't been initialized")
   }
   
   return false;
}

//******************************************************************************

void PthreadsConditionVariable::notifyOne() {
   if (m_initialized) {
      if (0 != ::pthread_cond_signal(&m_cond)) {
         LOG_ERROR("unable to signal on condition variable")
      }
   } else {
      LOG_ERROR("unable to notify because condition variable not initialized")
   }
}

//******************************************************************************

void PthreadsConditionVariable::notifyAll() {
   if (m_initialized) {
      if (0 != ::pthread_cond_broadcast(&m_cond)) {
         LOG_ERROR("unable to broadcast on condition variable")
      }
   } else {
      LOG_ERROR("unable to notify because condition variable not initialized")
   }
}

//******************************************************************************

const std::string& PthreadsConditionVariable::getName() const {
   return m_name;
}

//******************************************************************************
