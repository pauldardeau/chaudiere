// Copyright Paul Dardeau, SwampBits LLC 2017
// BSD License

#ifndef CHAUDIERE_ABSTRACTTESTCASE_H
#define CHAUDIERE_ABSTRACTTESTCASE_H

#include <string>


namespace chaudiere
{

class AbstractTestCase
{
public:
   virtual ~AbstractTestCase() {}

   virtual const std::string& getName() const = 0;
};

}

#endif
