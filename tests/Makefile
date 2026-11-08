# Copyright Paul Dardeau, SwampBits LLC 2014
# BSD License

CC = c++
CC_OPTS = -c -g -std=c++20 -pthread -I../src -I../poivre
#CC_OPTS = -c -std=c++20 -pthread -I../src -I../poivre

# see src/Makefile - same SANFLAGS convention, needed at link time here too
SANFLAGS =

EXE_NAME = test_chaudiere
LIB_NAMES = ../src/libchaudiere.so

# The shared library that TestDynamicLibrary opens, built from fixtures/DynamicLibraryFixture.cpp the way tests/CMakeLists.txt
# builds chaudiere_dynamic_library_fixture (hidden default visibility, -Wall -Wextra). It is test-only and is never installed.
# Linux makes a .so with -shared, macOS a .dylib with -dynamiclib.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
FIXTURE_SUFFIX = dylib
FIXTURE_LINK = -dynamiclib
else
FIXTURE_SUFFIX = so
FIXTURE_LINK = -shared
endif
FIXTURE_NAME = libchaudiere_dynamic_library_fixture.$(FIXTURE_SUFFIX)

POIVRE_OBJS = TestCase.o \
TestSuite.o \
TestRegistry.o

OBJS = MockSocket.o \
TestAutoPointer.o \
TestByteBuffer.o \
TestCharBuffer.o \
TestDateTime.o \
TestDynamicLibrary.o \
TestEpollServer.o \
TestFileLogger.o \
TestIniReader.o \
TestInvalidKeyException.o \
TestKeyValuePairs.o \
TestKqueueServer.o \
TestMutexLock.o \
TestNumberFormatException.o \
TestOptionParser.o \
TestOSUtils.o \
TestPthreadsConditionVariable.o \
TestPthreadsMutex.o \
TestPthreadsThreadingFactory.o \
TestRequestHandler.o \
TestServerSocket.o \
TestServiceInfo.o \
TestSocket.o \
TestSocketBoundedRead.o \
TestSocketConnectInterrupt.o \
TestSocketDescriptors.o \
TestSocketSigpipe.o \
TestSocketRequest.o \
TestSocketServer.o \
TestStdConditionVariable.o \
TestStdLogger.o \
TestStdMutex.o \
TestStdThread.o \
TestStdThreadingFactory.o \
TestStrUtils.o \
TestStringTokenizer.o \
TestSystemInfo.o \
TestSystemStats.o \
TestThread.o \
TestThreadInfo.o \
TestThreadPool.o \
TestThreadPoolQueue.o \
TestThreadPoolWorker.o \
TestThreadingFactory.o \
TestUtils.o \
Tests.o \
$(POIVRE_OBJS)

all : $(EXE_NAME)

clean :
	rm -f *.o
	rm -f $(EXE_NAME)
	rm -f $(FIXTURE_NAME)

$(EXE_NAME) : $(OBJS) $(ZLIB_OBJS) $(FIXTURE_NAME)
	$(CC) $(SANFLAGS) $(OBJS) -o $(EXE_NAME) $(LIB_NAMES) -lpthread -ldl

$(FIXTURE_NAME) : fixtures/DynamicLibraryFixture.cpp
	$(CC) -std=c++20 -fPIC -fvisibility=hidden -Wall -Wextra $(SANFLAGS) $(FIXTURE_LINK) $< -o $@

# The fixture's absolute path is given to this one source as a macro (TestDynamicLibrary.cpp refuses to compile without it).
# It is given here and not through CC_OPTS, which the CI jobs replace on the make command line. Quoting: the shell sees the
# whole -D argument as one word (single quotes), so a checkout path with spaces stays in one piece; the double quotes inside
# are part of the macro and make it a C++ string literal. A path containing ' " \ or $ is not supported.
TestDynamicLibrary.o : TestDynamicLibrary.cpp $(FIXTURE_NAME)
	$(CC) $(CC_OPTS) '-DCHAUDIERE_DYNAMIC_LIBRARY_FIXTURE_PATH="$(CURDIR)/$(FIXTURE_NAME)"' $< -o $@

$(POIVRE_OBJS) : %.o : ../poivre/%.cpp
	$(CC) $(CC_OPTS) $< -o $@

%.o : %.cpp
	$(CC) $(CC_OPTS) $< -o $@
