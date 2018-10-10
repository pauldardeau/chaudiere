// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_PTHREADSCONDITIONVARIABLE_H
#define CHAUDIERE_PTHREADSCONDITIONVARIABLE_H

#include <pthread.h>
#include <memory>
#include <string>

#include "ConditionVariable.h"

namespace chaudiere
{

/**
 *
 */
class PthreadsConditionVariable : public ConditionVariable
{
public:
   /**
    * Default constructor
    */
   PthreadsConditionVariable();

   PthreadsConditionVariable(const std::string& name);

   PthreadsConditionVariable(const PthreadsConditionVariable&) = delete;
   PthreadsConditionVariable& operator=(const PthreadsConditionVariable&) = delete;
   
   /**
    * Destructor
    */
   ~PthreadsConditionVariable();
   
   /**
    *
    * @param mutex
    * @return
    * @see Mutex()
    */
   virtual bool wait(std::unique_ptr<Mutex>& mutex);
   
   /**
    *
    */
   virtual void notifyOne();
   
   /**
    *
    */
   virtual void notifyAll();

   virtual const std::string& getName() const;

   
private:
   pthread_cond_t m_cond;
   bool m_initialized;
   std::string m_name;
};

}

#endif

