// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>

/*
#if defined(__FreeBSD__) || defined(__APPLE__)
#define HAVE_GCD 1
// libdispatch (core of Apple's Grand Central Dispatch)
#include <dispatch/dispatch.h>
#include <dispatch/queue.h>
#else
#undef HAVE_GCD
#endif
*/

#include "ThreadPoolDispatch.h"
#include "Runnable.h"
#include "Logger.h"
#include "BasicException.h"

using namespace chaudiere;

//******************************************************************************

ThreadPoolDispatch::ThreadPoolDispatch() :
   m_isRunning(false) {
   Logger::logInstanceCreate("ThreadPoolDispatch");
}

//******************************************************************************

ThreadPoolDispatch::~ThreadPoolDispatch() {
   Logger::logInstanceDestroy("ThreadPoolDispatch");
}

//******************************************************************************

bool ThreadPoolDispatch::start() {
   m_isRunning = true;
   return true;
}

//******************************************************************************

bool ThreadPoolDispatch::stop() {
   m_isRunning = false;
   return true;
}

//******************************************************************************

bool ThreadPoolDispatch::addRequest(Runnable* runnableRequest) {
   if (!m_isRunning || !runnableRequest) {
      return false;
   }
   
#ifdef HAVE_GCD
   dispatch_queue_t queue =
      dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
   
   dispatch_async(queue, ^{
      try {
         runnableRequest->run();
      } catch (const BasicException& be) {
         Logger::error("exception running request: " + be.whatString());
      } catch (const std::exception& e) {
         Logger::error("exception running request: " + std::string(e.what()));
      } catch (...) {
         Logger::error("unknown exception running request");
      }
      
      runnableRequest->notifyOnCompletion();
   });
   
   return true;
#else
   return false;
#endif
}

//******************************************************************************

