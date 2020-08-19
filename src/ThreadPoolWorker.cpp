// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>
#include <string.h>
#include <exception>

#include "ThreadPoolWorker.h"
#include "ThreadPoolQueue.h"
#include "ThreadingFactory.h"
#include "Thread.h"
#include "BasicException.h"
#include "Logger.h"

using namespace chaudiere;

//******************************************************************************

ThreadPoolWorker::ThreadPoolWorker(ThreadingFactory* threadingFactory,
                                   ThreadPoolQueue& queue,
                                   int workerId) :
   m_threadingFactory(threadingFactory),
   m_workerThread(nullptr),
   m_poolQueue(queue),
   m_workerId(workerId),
   m_isRunning(false) {
   LOG_INSTANCE_CREATE("ThreadPoolWorker")
}

//******************************************************************************

ThreadPoolWorker::~ThreadPoolWorker() {
   LOG_INSTANCE_DESTROY("ThreadPoolWorker")
}

//******************************************************************************

void ThreadPoolWorker::start() {
   if (!m_workerThread) {
      m_workerThread = m_threadingFactory->createThread(this, "threadpoolworker");
      if (m_workerThread) {
         m_workerThread->setPoolWorkerStatus(true);
         m_workerThread->setWorkerId(std::to_string(m_workerId));
         m_isRunning = true;
         m_workerThread->start();
      }
   }
}

//******************************************************************************

void ThreadPoolWorker::stop() {
   m_isRunning = false;
}

//******************************************************************************

void ThreadPoolWorker::run() {
   while (m_isRunning) {
      if (Logger::isLogging(LogLevel::Debug)) {
         char message[128];
         ::snprintf(message, 128, "poolQueue taking request on thread %d", m_workerId);
         LOG_DEBUG(message)
      }

      Runnable* runnable = m_poolQueue.takeRequest();
      if (runnable) {
         // has our thread been notified to shut down?
         if (!m_workerThread->isAlive()) {
            // put the request back on the front of the queue
            m_isRunning = false;
            continue;
         } else {
            // mark it
            runnable->setRunByThreadId(m_workerId);
            runnable->setRunByThreadWorkerId(m_workerThread->getWorkerId());
            
            try {
               runnable->run();
            } catch (const BasicException& be) {
               LOG_ERROR("run method of runnable threw exception: " + be.whatString())
            } catch (const std::exception& e) {
               LOG_ERROR("run method of runnable threw exception: " + std::string(e.what()))
            } catch (...) {
               LOG_ERROR("run method of runnable threw exception")
            }

            runnable->notifyOnCompletion();

            if (Logger::isLogging(LogLevel::Debug)) {
               char message[128];
               ::snprintf(message, 128,
                             "ending processing request on thread %d",
                             m_workerId);
               LOG_DEBUG(message)
            }
         }

	 if (runnable->isAutoDelete()) {
            delete runnable;
         }
      }
   }
}

//******************************************************************************


