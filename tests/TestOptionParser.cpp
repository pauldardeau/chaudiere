// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "OptionParser.h"
#include "TestOptionParser.h"
#include "Runnable.h"

using namespace chaudiere;

//******************************************************************************

TestOptionParser::TestOptionParser() :
   TestSuite("TestOptionParser") {
}

//******************************************************************************

void TestOptionParser::runTests() {
   testConstructor();
   testCopyConstructor();
   testAssignmentOperator();
   testAddFlag();
   testAddOption();
   testHasOption();
   testGetOptionValue();
   testHasFlag();
   testParseArgs();
}

//******************************************************************************

void TestOptionParser::testConstructor() {
   TEST_CASE("testConstructor");

   OptionParser op;
   requireFalse(op.hasOption("foo"), "empty parser has no options");
}

//******************************************************************************

void TestOptionParser::testCopyConstructor() {
   TEST_CASE("testCopyConstructor");

   OptionParser opEmpty;
   OptionParser opCopyEmpty(opEmpty);
   requireFalse(opCopyEmpty.hasOption("foo"),
                "copy ctor of empty parser still empty");

   OptionParser opNonEmpty;
   std::string option = "prefix";
   std::string defaultOptionValue = "myprefix";
   std::string flag = "debugging";
   opNonEmpty.addOption(option, defaultOptionValue);
   opNonEmpty.addFlagOption(flag);
   OptionParser opCopyNonEmpty(opNonEmpty);
   require(opCopyNonEmpty.hasOption(option),
           "copy ctor should retain option");
   requireStringEquals(defaultOptionValue,
                       opCopyNonEmpty.getOptionValue(option),
                       "copy ctor should retain default value");
   require(opCopyNonEmpty.acceptsFlag(flag),
           "copy ctor should retain boolean option");
}

//******************************************************************************

void TestOptionParser::testAssignmentOperator() {
   TEST_CASE("testAssignmentOperator");

   OptionParser opEmpty;
   OptionParser opCopyEmpty;
   opCopyEmpty = opEmpty;
   requireFalse(opCopyEmpty.hasOption("foo"),
                "assignment of empty parser still empty");

   OptionParser opNonEmpty;
   std::string nonBoolOption = "prefix";
   std::string flagOption = "debugging";
   std::string nonBoolVar = "prefix_var";
   opNonEmpty.addOption(nonBoolOption, nonBoolVar);
   opNonEmpty.addFlagOption(flagOption);
   OptionParser opCopyNonEmpty;
   opCopyNonEmpty = opNonEmpty;
   require(opCopyNonEmpty.hasOption(nonBoolOption),
           "assignment should retain option");
   require(opCopyNonEmpty.acceptsFlag(flagOption),
           "assignment should retain boolean option");
}

//******************************************************************************

void TestOptionParser::testAddFlag() {
   TEST_CASE("testAddFlag");

   OptionParser op;
   std::string optionName = "";
   requireFalse(op.addFlagOption(optionName),
                "add invalid option return false");
   optionName = "debugging";
   require(op.addFlagOption(optionName),
           "add valid option return true");
}

//******************************************************************************

void TestOptionParser::testAddOption() {
   TEST_CASE("testAddOption");

   OptionParser op;
   std::string option;
   std::string option_var;
   requireFalse(op.addOption(option, option_var),
                "add invalid option return false");
   option = "logger";
   option_var = "logger_var";
   require(op.addOption(option, option_var), "add valid option return true");
}

//******************************************************************************

void TestOptionParser::testHasOption() {
   TEST_CASE("testHasOption");

   OptionParser op;
   requireFalse(op.hasOption("foo"),
                "test for non-existing option return false");
   std::string option = "logger";
   std::string option_var = "logger_var";
   op.addOption(option, option_var);
   require(op.hasOption(option), "test for existing option return true");
}

//******************************************************************************

void TestOptionParser::testGetOptionValue() {
   TEST_CASE("testGetOptionValue");

   int argc;
   std::string flag = "debug";
   std::string option = "logger";
   std::string optionValue = "MyLogger";

   {
      const char* argv1[] = {"myprog", NULL};
      argc = sizeof(argv1) / sizeof(char*) - 1;
      OptionParser op;
      op.parseArgs(argc, argv1);
      requireFalse(op.hasFlag(flag), "test non-added flag option");
      requireFalse(op.acceptsFlag(flag), "test non-added option");
      op.addFlagOption(flag);
      require(op.acceptsFlag(flag), "test added flag option");
   }

   {
      const char* argv2[] = {"myprog", "debug", "logger", "MyLogger", NULL};
      argc = sizeof(argv2) / sizeof(char*) - 1;
      OptionParser op;
      op.addFlagOption(flag);
      op.addOption(option, std::string(""));
      op.parseArgs(argc, argv2);
      require(op.hasFlag(flag), "test added flag option");
      require(op.hasOption(option), "test added option");
      requireStringEquals(optionValue, op.getOptionValue(option),
                          "option value matches command line value");
   }

   //TODO: test non-existing option (InvalidKeyException)
   {
      class GetInvalidOption : public poivre::Runnable {
         public:
            void run() {
               OptionParser op;
	       op.getOptionValue("nonExistingKey");
	    }
      };
      poivre::Runnable* runnable = new GetInvalidOption;
      requireException("InvalidKeyException",
		       runnable,
		       std::string("test getOptionValue for invalid (non-existing) option"));
   }
}

//******************************************************************************

void TestOptionParser::testHasFlag() {
   TEST_CASE("testHasFlag");

   {
      OptionParser op;
      requireFalse(op.hasFlag("foo"),
                   "test for non-existing option return false");
   }

   {
      const char* argv[] = {"myprog", "debugging", NULL}; 
      int argc = sizeof(argv) / sizeof(char*) - 1;
      OptionParser op;
      std::string option_name = "debugging";
      op.addFlagOption(option_name);
      op.parseArgs(argc, argv);
      require(op.hasFlag(option_name),
              "test for existing option return true");
   }
}

//******************************************************************************

void TestOptionParser::testParseArgs() {
   TEST_CASE("testParseArgs");

   int argc;
   const char* argv_no_opts[] = {"", NULL};
   argc = sizeof(argv_no_opts) / sizeof(char*) - 1;
   //TODO: implement testParseArgs 
}

//******************************************************************************

