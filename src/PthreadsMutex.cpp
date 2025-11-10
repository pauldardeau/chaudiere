// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <errno.h>

#include "PthreadsMutex.h"
#include "BasicException.h"
#include "Logger.h"

static const std::string EMPTY_STRING = "";

using namespace chaudiere;

//******************************************************************************

PthreadsMutex::PthreadsMutex() :
   m_mutexName(EMPTY_STRING),
   m_haveValidMutex(false),
   m_isLocked(false) {

   initialize();
}

//******************************************************************************

PthreadsMutex::PthreadsMutex(const std::string& mutexName) :
   m_mutexName(mutexName),
   m_haveValidMutex(false),
   m_isLocked(false) {

   initialize();
}

//******************************************************************************

PthreadsMutex::~PthreadsMutex() {
   LOG_INSTANCE_DESTROY("PthreadsMutex")

   if (m_haveValidMutex) {
      if (m_isLocked) {
         unlock();
      }
      ::pthread_mutex_destroy(&m_mutex);
   }
}

//******************************************************************************

void PthreadsMutex::initialize() {
   LOG_INSTANCE_CREATE("PthreadsMutex")
   char buffer[128];
   int rc;

   pthread_mutexattr_t attr;
   rc = ::pthread_mutexattr_init(&attr);
   if (0 == rc) {
      rc = ::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

      if (0 == rc) {
         rc = ::pthread_mutex_init(&m_mutex, &attr);

         ::pthread_mutexattr_destroy(&attr);

         if (0 == rc) {
            m_haveValidMutex = true;
         } else {
            snprintf(buffer, 128, "unable to create pthreads mutex, rc=%d", rc);
            LOG_ERROR(buffer)
            throw BasicException(buffer);
         }
      } else {
         snprintf(buffer, 128, "unable to set pthreads mutex type, rc=%d", rc);
         LOG_ERROR(buffer)
         throw BasicException(buffer);
      }
   } else {
      snprintf(buffer, 128, "unable to initialize mutex attributes, rc=%d", rc);
      LOG_ERROR(buffer)
      throw BasicException(buffer);
   }
}

//******************************************************************************

bool PthreadsMutex::unlock() {
   if (m_haveValidMutex) { // && m_isLocked) {
      const int rc = ::pthread_mutex_unlock(&m_mutex);
      if (0 == rc) {
         m_isLocked = false;
      } else {
         printf("error: mutex unlock failed, name='%s', rc=%d\n", m_mutexName.c_str(), rc);
      }
      return !m_isLocked;
   } else {
      if (!m_haveValidMutex) {
         printf("error: mutex unlock called, missing valid mutex\n");
      }

      return false;
   }
}

//******************************************************************************

bool PthreadsMutex::lock() {
   if (m_haveValidMutex) {
      const int rc = ::pthread_mutex_lock(&m_mutex);
      if (0 == rc) {
         m_isLocked = true;
      } else {
         printf("mutex lock failed, name='%s', rc=%d\n", m_mutexName.c_str(), rc);
         std::string errorCode;

         switch(rc) {
            case EINVAL:
               errorCode = "EINVAL";
               break;
            case EAGAIN:
               errorCode = "EAGAIN";
               break;
            case EDEADLK:
               errorCode = "EDEADLK";
               break;
         }

         std::string buffer = "unable to lock mutex, pthread_mutex_lock rc=";
         buffer += errorCode;
         LOG_ERROR(buffer)
      }

      return m_isLocked;
   } else {
      printf("locking not attempted, no valid mutex, name='%s'\n", m_mutexName.c_str());
      return false;
   }
}

//******************************************************************************

bool PthreadsMutex::isLocked() const {
   return m_haveValidMutex && m_isLocked;
}

//******************************************************************************

bool PthreadsMutex::haveValidMutex() const {
   return m_haveValidMutex;
}

//******************************************************************************

const std::string& PthreadsMutex::getName() const {
   return m_mutexName;
}

//******************************************************************************

