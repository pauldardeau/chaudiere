// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_TESTSERVICEINFO_H
#define CHAUDIERE_TESTSERVICEINFO_H

#include "TestSuite.h"

namespace chaudiere
{

class TestServiceInfo : public poivre::TestSuite
{
protected:
   void runTests();

   void testDefaultConstructor();
   void testConstructor();
   void testCopyConstructor();
   void testAssignmentOperator();
   void testServiceName();
   void testHost();
   void testPort();
   void testSetServiceName();
   void testSetHost();
   void testSetPort();
   void testGetUniqueIdentifier();
   void testSetPersistentConnection();
   void testGetPersistentConnection();

public:
   TestServiceInfo();

};

}

#endif
