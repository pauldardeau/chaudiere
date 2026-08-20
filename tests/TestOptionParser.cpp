// Copyright Paul Dardeau, SwampBits LLC 2015
// BSD License

#include "OptionParser.h"
#include "TestOptionParser.h"

using namespace chaudiere;

//******************************************************************************

TestOptionParser::TestOptionParser() :
   poivre::TestSuite("TestOptionParser") {
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
   const char* argv1[] = {"myprog", nullptr};
   argc = sizeof(argv1) / sizeof(char*) - 1;
   OptionParser op;
   op.parseArgs(argc, argv1);
   std::string flag = "debug";
   std::string option = "logger";
   std::string optionValue = "MyLogger";
   requireFalse(op.hasFlag(flag), "test non-added flag option");
   requireFalse(op.acceptsFlag(flag), "test non-added option");
   op.addFlagOption(flag);
   require(op.acceptsFlag(flag), "test added flag option");

   // parseArgs() treats argv[0] as the program name and skips it, so this
   // needs a leading placeholder before the actual "debug logger MyLogger"
   // tokens (parseArgs() populates hasOption()/getOptionValue() directly
   // from any "key value" pair on the command line -- addOption() isn't
   // required first, it's only for supplying defaults ahead of parsing)
   const char* argv2[] = {"myprog", "debug", "logger", "MyLogger", nullptr};
   argc = sizeof(argv2) / sizeof(char*) - 1;
   op.parseArgs(argc, argv2);
   require(op.hasFlag(flag), "test added flag option");
   require(op.hasOption(option), "test added option");
   requireStringEquals(optionValue, op.getOptionValue(option),
                       "option value matches command line value");

   // requesting the value of an option that was never added or parsed
   // should throw InvalidKeyException rather than returning garbage
   class GetMissingOptionValue : public poivre::Runnable {
   public:
      explicit GetMissingOptionValue(const OptionParser& op) : m_op(op) {}
      void run() override {
         m_op.getOptionValue("neverAdded");
      }
   private:
      const OptionParser& m_op;
   };
   requireException("InvalidKeyException", new GetMissingOptionValue(op));
}

//******************************************************************************

void TestOptionParser::testHasFlag() {
   TEST_CASE("testHasFlag");

   OptionParser op;
   requireFalse(op.hasFlag("foo"),
                "test for non-existing option return false");

   // hasFlag() reflects flags actually seen while parsing (m_flagsPresent),
   // which is distinct from acceptsFlag() (registered via addFlagOption,
   // tested separately) -- merely registering a flag shouldn't make
   // hasFlag() true until parseArgs() has actually seen it
   std::string option_name = "debugging";
   op.addFlagOption(option_name);
   requireFalse(op.hasFlag(option_name),
                "registering a flag should not by itself make hasFlag true");

   const char* argv[] = {"myprog", "debugging", nullptr};
   const int argc = sizeof(argv) / sizeof(char*) - 1;
   op.parseArgs(argc, argv);
   require(op.hasFlag(option_name),
           "test for existing option return true");
}

//******************************************************************************

void TestOptionParser::testParseArgs() {
   TEST_CASE("testParseArgs");

   // no arguments beyond the program name: nothing should be present
   OptionParser opNoArgs;
   opNoArgs.addFlagOption("verbose");
   const char* argvNoOpts[] = {"myprog", nullptr};
   int argc = sizeof(argvNoOpts) / sizeof(char*) - 1;
   opNoArgs.parseArgs(argc, argvNoOpts);
   requireFalse(opNoArgs.hasFlag("verbose"), "a flag not present on the command line should not be flagged as present");

   // a registered flag and a key/value pair together
   OptionParser op;
   op.addFlagOption("verbose");
   op.addOption("config", "default.ini");

   const char* argv[] = {"myprog", "verbose", "config", "custom.ini", nullptr};
   argc = sizeof(argv) / sizeof(char*) - 1;
   op.parseArgs(argc, argv);

   require(op.hasFlag("verbose"), "flag present on the command line should be detected");
   require(op.hasOption("config"), "option present on the command line should be detected");
   requireStringEquals("custom.ini", op.getOptionValue("config"), "option value should reflect the command-line value, overriding the default");
}

//******************************************************************************

