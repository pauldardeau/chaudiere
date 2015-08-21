// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_PTHREADSCONDITIONVARIABLE_H
#define CHAUDIERE_PTHREADSCONDITIONVARIABLE_H

#include <pthread.h>

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
   virtual bool wait(Mutex* mutex) noexcept override;
   
   /**
    *
    */
   virtual void notifyOne() noexcept override;
   
   /**
    *
    */
   virtual void notifyAll() noexcept override;

   
private:
   // disallow copies
   PthreadsConditionVariable(const PthreadsConditionVariable&);
   PthreadsConditionVariable& operator=(const PthreadsConditionVariable&);

   pthread_cond_t m_cond;
   bool m_initialized;
};

}

#endif

