// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef TESTCASE_H
#define TESTCASE_H

#include <string>

class TestSuite;


class TestCase
{
private:
   TestSuite& m_testSuite;
   std::string m_testCaseName;
   
   TestCase(const TestCase& copy);
   TestCase& operator=(const TestCase& copy);

   
public:
   TestCase(TestSuite& testSuite, const std::string& testCaseName);
   virtual ~TestCase();
   
   const std::string& getName() const;
   
};

#endif
