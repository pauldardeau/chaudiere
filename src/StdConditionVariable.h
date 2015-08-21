// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_STDCONDITIONVARIABLE_H
#define CHAUDIERE_STDCONDITIONVARIABLE_H

#include <condition_variable>

#include "ConditionVariable.h"

namespace chaudiere
{

/**
 *
 */
class StdConditionVariable : public ConditionVariable
{
public:
   /**
    * Default constructor
    */
   StdConditionVariable();
   
   /**
    * Destructor
    */
   ~StdConditionVariable();
   
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
   StdConditionVariable(const StdConditionVariable&);
   StdConditionVariable& operator=(const StdConditionVariable&);

   std::condition_variable m_cond;
};

}

#endif
