// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include <stdio.h>

#include "TestIniReader.h"
#include "IniReader.h"
#include "KeyValuePairs.h"

using namespace chaudiere;
using namespace poivre;

//******************************************************************************

TestIniReader::TestIniReader() :
   TestSuite("TestIniReader") {
}

//******************************************************************************

void TestIniReader::runTests() {
   setupSuite();

   testConstructor();
   testReadSection();
   testGetSectionKeyValue();
   testHasSection();

   tearDownSuite();
}

//******************************************************************************

void TestIniReader::setupSuite() {
   m_filePath = getTempFile();
   printf("TestIniReader::setupSuite, temp file ='%s'\n", m_filePath.c_str());

   if (!m_filePath.empty()) {
      FILE *f = ::fopen(m_filePath.c_str(), "wt");
      if (f != nullptr) {
         ::fprintf(f, "# this is a comment line\n");
         ::fprintf(f, "[stooges]\n");
         ::fprintf(f, "stooge1 = Moe\n");
         ::fprintf(f, "stooge2 = Larry\n");
         ::fprintf(f, "stooge3 = Curly\n");
         ::fprintf(f, "\n");
         ::fprintf(f, "# [birds]\n");
         ::fprintf(f, "# state_bird = Pelican\n");
         ::fprintf(f, "# woodeater = Woodpecker\n");
         ::fprintf(f, "# nofly = Ostrich\n");
         ::fprintf(f, "\n");
         ::fprintf(f, "[artists]\n");
         ::fprintf(f, "painter1 = Monet\n");
         ::fprintf(f, "painter2 = Van Gogh\n");
         ::fprintf(f, "painter3 = Rembrandt\n");
         ::fprintf(f, "\n");

         ::fclose(f);
      }
   }
}

//******************************************************************************

void TestIniReader::tearDownSuite() {
   if (!m_filePath.empty()) {
      deleteFile(m_filePath);
      m_filePath = "";
   }
}

//******************************************************************************

class IniReaderCtorRunner : public Runnable {
public:
   explicit IniReaderCtorRunner(const std::string& arg) :
      m_arg(arg) {
   }

   IniReaderCtorRunner(const IniReaderCtorRunner& copy) :
      m_arg(copy.m_arg) {
   }

   IniReaderCtorRunner& operator=(const IniReaderCtorRunner& copy) {
      if (this == &copy) {
         return *this;
      }

      m_arg = copy.m_arg;

      return *this;
   }

   void run() {
      IniReader reader(m_arg);
   }

private:
   std::string m_arg;
};

void TestIniReader::testConstructor() {
   TEST_CASE("testConstructor");

   //IniReader(const std::string& iniFile);
   IniReader reader(m_filePath);

   //test with non-existing file
   requireException("BasicException",
                    new IniReaderCtorRunner("ljsfa/asdflkj/sadflkjx"),
                    "non-existing file");
}

//******************************************************************************

void TestIniReader::testReadSection() {
   TEST_CASE("testReadSection");

   //bool readSection(const std::string& section, KeyValuePairs& mapSectionValues) const;
   IniReader reader(m_filePath);

   std::string section = "stooges";
   require(reader.hasSection(section), "existing section should be known");
   KeyValuePairs kvpStooges;
   require(reader.readSection(section, kvpStooges), "read existing section");
   require(3 == kvpStooges.size(), "should return 3 pairs");

   KeyValuePairs kvpBirds;
   section = "birds";
   requireFalse(reader.hasSection(section), "non-existing section should return false");
   requireFalse(reader.readSection(section, kvpBirds), "read of non-existing section should return false");
}

//******************************************************************************

void TestIniReader::testGetSectionKeyValue() {
   TEST_CASE("testGetSectionKeyValue");

   //bool getSectionKeyValue(const std::string& section,
   //                         const std::string& key,
   //                         std::string& value) const;
   IniReader reader(m_filePath);

   std::string section = "stooges";
   std::string value;
   require(reader.hasSection(section), "existing section should be known");
   require(reader.getSectionKeyValue(section, "stooge1", value), "should be able to read value");
   requireStringEquals("Moe", value, "stooge1 is Moe");
   requireFalse(reader.getSectionKeyValue(section, "stooge4", value), "should not be able to read non-existing value");
   requireFalse(reader.getSectionKeyValue("birds", "state_bird", value), "should not be able to read value for non-existing section");
}

//******************************************************************************

void TestIniReader::testHasSection() {
   TEST_CASE("testHasSection");

   //bool hasSection(const std::string& section) const;
   IniReader reader(m_filePath);
   require(reader.hasSection("stooges"), "existing section should be known");
   requireFalse(reader.hasSection("birds"), "non-existent section should return false");
}

//******************************************************************************

