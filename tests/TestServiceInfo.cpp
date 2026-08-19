// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestServiceInfo.h"
#include "ServiceInfo.h"

using namespace chaudiere;

//******************************************************************************

TestServiceInfo::TestServiceInfo() :
   poivre::TestSuite("TestServiceInfo") {
}

//******************************************************************************

void TestServiceInfo::runTests() {
   testDefaultConstructor();
   testConstructor();
   testCopyConstructor();
   testAssignmentOperator();
   testServiceName();
   testHost();
   testPort();
   testSetServiceName();
   testSetHost();
   testSetPort();
   testGetUniqueIdentifier();
   testSetPersistentConnection();
   testGetPersistentConnection();
}

//******************************************************************************

void TestServiceInfo::testDefaultConstructor() {
   TEST_CASE("testDefaultConstructor");

   ServiceInfo si;
   require(si.serviceName().empty(), "default serviceName should be empty");
   require(si.host().empty(), "default host should be empty");
   require(0 == si.port(), "default port should be 0");
   requireFalse(si.getPersistentConnection(), "default persistent connection should be false");
}

//******************************************************************************

void TestServiceInfo::testConstructor() {
   TEST_CASE("testConstructor");

   ServiceInfo si("echo_service", "127.0.0.1", 9000);
   requireStringEquals("echo_service", si.serviceName(), "serviceName should match constructor argument");
   requireStringEquals("127.0.0.1", si.host(), "host should match constructor argument");
   require(9000 == si.port(), "port should match constructor argument");
   requireFalse(si.getPersistentConnection(), "persistent connection should default to false");
}

//******************************************************************************

void TestServiceInfo::testCopyConstructor() {
   TEST_CASE("testCopyConstructor");

   ServiceInfo original("echo_service", "127.0.0.1", 9000);
   original.setPersistentConnection(true);

   ServiceInfo copy(original);
   requireStringEquals(original.serviceName(), copy.serviceName(), "copy should have the same serviceName");
   requireStringEquals(original.host(), copy.host(), "copy should have the same host");
   require(original.port() == copy.port(), "copy should have the same port");
   require(original.getPersistentConnection() == copy.getPersistentConnection(), "copy should have the same persistent connection setting");
}

//******************************************************************************

void TestServiceInfo::testAssignmentOperator() {
   TEST_CASE("testAssignmentOperator");

   ServiceInfo source("echo_service", "127.0.0.1", 9000);
   source.setPersistentConnection(true);

   ServiceInfo target;
   target = source;

   requireStringEquals("echo_service", target.serviceName(), "target should have the same serviceName after assignment");
   requireStringEquals("127.0.0.1", target.host(), "target should have the same host after assignment");
   require(9000 == target.port(), "target should have the same port after assignment");
   require(target.getPersistentConnection(), "target should have the same persistent connection setting after assignment");

   // self-assignment should be a safe no-op
   target = target;
   requireStringEquals("echo_service", target.serviceName(), "self-assignment should not corrupt state");
}

//******************************************************************************

void TestServiceInfo::testServiceName() {
   TEST_CASE("testServiceName");

   ServiceInfo si("myService", "host1", 1234);
   requireStringEquals("myService", si.serviceName(), "serviceName should return the constructor-specified name");
}

//******************************************************************************

void TestServiceInfo::testHost() {
   TEST_CASE("testHost");

   ServiceInfo si("myService", "host1", 1234);
   requireStringEquals("host1", si.host(), "host should return the constructor-specified host");
}

//******************************************************************************

void TestServiceInfo::testPort() {
   TEST_CASE("testPort");

   ServiceInfo si("myService", "host1", 1234);
   require(1234 == si.port(), "port should return the constructor-specified port");
}

//******************************************************************************

void TestServiceInfo::testSetServiceName() {
   TEST_CASE("testSetServiceName");

   ServiceInfo si;
   si.setServiceName("updatedService");
   requireStringEquals("updatedService", si.serviceName(), "serviceName should reflect setServiceName");
}

//******************************************************************************

void TestServiceInfo::testSetHost() {
   TEST_CASE("testSetHost");

   ServiceInfo si;
   si.setHost("updatedHost");
   requireStringEquals("updatedHost", si.host(), "host should reflect setHost");
}

//******************************************************************************

void TestServiceInfo::testSetPort() {
   TEST_CASE("testSetPort");

   ServiceInfo si;
   si.setPort(5555);
   require(5555 == si.port(), "port should reflect setPort");
}

//******************************************************************************

void TestServiceInfo::testGetUniqueIdentifier() {
   TEST_CASE("testGetUniqueIdentifier");

   ServiceInfo si("myService", "192.168.1.5", 8080);
   requireStringEquals("192.168.1.5|8080", si.getUniqueIdentifier(), "getUniqueIdentifier should combine host and port with a '|' delimiter");
}

//******************************************************************************

void TestServiceInfo::testSetPersistentConnection() {
   TEST_CASE("testSetPersistentConnection");

   ServiceInfo si;
   si.setPersistentConnection(true);
   require(si.getPersistentConnection(), "getPersistentConnection should reflect setPersistentConnection(true)");
   si.setPersistentConnection(false);
   requireFalse(si.getPersistentConnection(), "getPersistentConnection should reflect setPersistentConnection(false)");
}

//******************************************************************************

void TestServiceInfo::testGetPersistentConnection() {
   TEST_CASE("testGetPersistentConnection");

   ServiceInfo si;
   requireFalse(si.getPersistentConnection(), "getPersistentConnection should default to false");
}

//******************************************************************************
