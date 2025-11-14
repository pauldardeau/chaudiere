// Copyright Paul Dardeau, SwampBits LLC 2017
// BSD License

#ifndef CHAUDIERE_ABSTRACTTESTSUITE_H
#define CHAUDIERE_ABSTRACTTESTSUITE_H

#include <string>


namespace chaudiere
{

class AbstractTestSuite
{
public:
   virtual ~AbstractTestSuite() {}

   virtual void startingTestCase(const AbstractTestCase& testCase) = 0;
   virtual void endingTestCase(const AbstractTestCase& testCase) = 0;

};

}

#endif
