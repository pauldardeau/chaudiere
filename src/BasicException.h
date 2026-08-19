// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_BASICEXCEPTION_H
#define CHAUDIERE_BASICEXCEPTION_H

#include <string>

#include "../poivre/BasicException.h"


namespace chaudiere
{

/**
 * BasicException is chaudiere's own exception type, kept under the
 * chaudiere namespace (and as the base of NumberFormatException /
 * InvalidKeyException) so existing production code is unaffected, but it
 * derives from poivre::BasicException so it's still catchable as one -
 * e.g. by poivre::TestSuite::requireException() in the test suite.
 */
class BasicException : public poivre::BasicException
{
public:
   /**
    * Constructs a BasicException
    * @param what the reason for the exception
    */
   explicit BasicException(const char* what) :
      poivre::BasicException(what) {
   }

   /**
    * Constructs a BasicException
    * @param what the reason for the exception
    */
   explicit BasicException(const std::string& what) :
      poivre::BasicException(what) {
   }

   /**
    * Copy constructor
    * @param copy the source of the copy
    */
   BasicException(const BasicException& copy) :
      poivre::BasicException(copy) {
   }

   /**
    * Destructor
    */
   virtual ~BasicException() noexcept {}
};

}

#endif

