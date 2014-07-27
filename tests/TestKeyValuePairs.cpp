// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestKeyValuePairs.h"
#include "KeyValuePairs.h"

//******************************************************************************

TestKeyValuePairs::TestKeyValuePairs() :
   TestSuite("TestKeyValuePairs")
{
}

//******************************************************************************

void TestKeyValuePairs::runTests()
{
   testConstructor();
   testCopyConstructor();
   testMoveConstructor();
   testAssignmentCopy();
   testAssignmentMove();
   testGetKeys();
   testHasKey();
   testGetValue();
   testAddPair();
   testRemovePair();
   testClear();
   testSize();
   testEmpty();
}

//******************************************************************************

void TestKeyValuePairs::testConstructor()
{
   TEST_CASE("testConstructor");
   
   KeyValuePairs kvp;
   require(kvp.empty(), "empty should be empty");
   require(kvp.size() == 0, "empty should have count of 0");
   requireFalse(kvp.hasKey("testKey"), "empty should have no key");
}

//******************************************************************************

void TestKeyValuePairs::testCopyConstructor()
{
   TEST_CASE("testCopyConstructor");
   
   KeyValuePairs kvp;
   kvp.addPair("stooge1", "Moe");
   kvp.addPair("stooge2", "Larry");
   kvp.addPair("stooge3", "Curly");
   
   KeyValuePairs kvpCopy(kvp);
   requireFalse(kvpCopy.empty(), "non-empty copy");
   require(3 == kvpCopy.size(), "3 pairs in copy");
}

//******************************************************************************

void TestKeyValuePairs::testMoveConstructor()
{
   TEST_CASE("testMoveConstructor");
}

//******************************************************************************

void TestKeyValuePairs::testAssignmentCopy()
{
   TEST_CASE("testAssignmentCopy");

   KeyValuePairs kvp;
   kvp.addPair("stooge1", "Moe");
   kvp.addPair("stooge2", "Larry");
   kvp.addPair("stooge3", "Curly");
   
   KeyValuePairs kvpCopy;
   kvpCopy = kvp;
   requireFalse(kvpCopy.empty(), "non-empty copy");
   require(3 == kvpCopy.size(), "3 pairs in copy");
}

//******************************************************************************

void TestKeyValuePairs::testAssignmentMove()
{
   TEST_CASE("testAssignmentMove");
   
}

//******************************************************************************

void TestKeyValuePairs::testGetKeys()
{
   TEST_CASE("testGetKeys");
   
   std::vector<std::string> keys;
   KeyValuePairs kvp;
   kvp.getKeys(keys);
   require(keys.empty(), "no keys");
   
   kvp.addPair("stooge1", "Moe");
   kvp.addPair("stooge2", "Larry");
   kvp.addPair("stooge3", "Curly");

   kvp.getKeys(keys);
   require(3 == keys.size(), "3 keys");
}

//******************************************************************************

void TestKeyValuePairs::testHasKey()
{
   TEST_CASE("testHasKey");

   KeyValuePairs kvp;
   requireFalse(kvp.hasKey("stooge1"), "no pairs");
   kvp.addPair("stooge1", "Moe");
   require(kvp.hasKey("stooge1"), "hasKey with 1 pair");
}

//******************************************************************************

void TestKeyValuePairs::testGetValue()
{
   TEST_CASE("testGetValue");

   KeyValuePairs kvp;
   kvp.addPair("stooge1", "Moe");

   requireStringEquals("Moe", kvp.getValue("stooge1"), "getValue");
}

//******************************************************************************

void TestKeyValuePairs::testAddPair()
{
   TEST_CASE("testAddPair");

   KeyValuePairs kvp;
   require(kvp.empty(), "empty");

   kvp.addPair("stooge1", "Moe");
   kvp.addPair("stooge2", "Larry");
   kvp.addPair("stooge3", "Curly");
   
   require(3 == kvp.size(), "3 pairs");
}

//******************************************************************************

void TestKeyValuePairs::testRemovePair()
{
   TEST_CASE("testRemovePair");

   KeyValuePairs kvp;
   kvp.addPair("stooge1", "Moe");
   kvp.addPair("stooge2", "Larry");
   kvp.addPair("stooge3", "Curly");

   kvp.removePair("stooge2");
   require(2 == kvp.size(), "2 pairs");
}

//******************************************************************************

void TestKeyValuePairs::testClear()
{
   TEST_CASE("testClear");
   
   KeyValuePairs kvp;
   kvp.addPair("stooge1", "Moe");
   kvp.addPair("stooge2", "Larry");
   kvp.addPair("stooge3", "Curly");

   requireFalse(kvp.empty(), "non-empty should be non-empty");
   kvp.clear();
   require(kvp.empty(), "clear should result in empty");
}

//******************************************************************************

void TestKeyValuePairs::testSize()
{
   TEST_CASE("testSize");

   KeyValuePairs kvp;
   kvp.addPair("stooge1", "Moe");
   kvp.addPair("stooge2", "Larry");
   kvp.addPair("stooge3", "Curly");

   require(3 == kvp.size(), "3 pairs");
}

//******************************************************************************

void TestKeyValuePairs::testEmpty()
{
   TEST_CASE("testEmpty");

   KeyValuePairs kvp;
   require(kvp.empty(), "empty should be empty");

   kvp.addPair("stooge1", "Moe");
   kvp.addPair("stooge2", "Larry");
   kvp.addPair("stooge3", "Curly");
   
   requireFalse(kvp.empty(), "non-empty should be non-empty");
}

//******************************************************************************

