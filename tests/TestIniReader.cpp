// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#include "TestIniReader.h"
#include "IniReader.h"
#include "KeyValuePairs.h"

//******************************************************************************

TestIniReader::TestIniReader() :
   TestSuite("TestIniReader")
{
}

//******************************************************************************

void TestIniReader::runTests()
{
   setupSuite();
   
   testConstructor();
   testReadSection();
   testGetSectionKeyValue();
   testHasSection();
   
   tearDownSuite();
}

//******************************************************************************

void TestIniReader::setupSuite()
{
   m_filePath = getTempFile();
   
   if (!m_filePath.empty()) {
      FILE *f = std::fopen(m_filePath.c_str(), "wt");
      if (f != nullptr) {
         std::fprintf(f, "# this is a comment line\n");
         std::fprintf(f, "[stooges]\n");
         std::fprintf(f, "stooge1 = Moe\n");
         std::fprintf(f, "stooge2 = Larry\n");
         std::fprintf(f, "stooge3 = Curly\n");
         std::fprintf(f, "\n");
         std::fprintf(f, "# [birds]\n");
         std::fprintf(f, "# state_bird = Pelican\n");
         std::fprintf(f, "# woodeater = Woodpecker\n");
         std::fprintf(f, "# nofly = Ostrich\n");
         std::fprintf(f, "\n");
         std::fprintf(f, "[artists]\n");
         std::fprintf(f, "painter1 = Monet\n");
         std::fprintf(f, "painter2 = Van Gogh\n");
         std::fprintf(f, "painter3 = Rembrandt\n");
         std::fprintf(f, "\n");

         std::fclose(f);
      }
   }
}

//******************************************************************************

void TestIniReader::tearDownSuite()
{
   if (!m_filePath.empty()) {
      deleteFile(m_filePath);
      m_filePath = "";
   }
}

//******************************************************************************

void TestIniReader::testConstructor()
{
   TEST_CASE("testConstructor");

   //IniReader(const std::string& iniFile);
   IniReader reader(m_filePath);
   
   //TODO: test with non-existing file
}

//******************************************************************************

void TestIniReader::testReadSection()
{
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

void TestIniReader::testGetSectionKeyValue()
{
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

void TestIniReader::testHasSection()
{
   TEST_CASE("testHasSection");
   
   //bool hasSection(const std::string& section) const;
   IniReader reader(m_filePath);
   require(reader.hasSection("stooges"), "existing section should be known");
   requireFalse(reader.hasSection("birds"), "non-existent section should return false");
}

//******************************************************************************

