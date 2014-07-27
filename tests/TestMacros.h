// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTMACROS_H
#define CHAUDIERE_TESTMACROS_H

#include "TestCase.h"


#define TEST_CASE(testCaseName) \
chaudiere::TestCase macroTestCase(*this,testCaseName);


#endif

