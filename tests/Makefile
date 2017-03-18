# Copyright Paul Dardeau, SwampBits LLC 2014
# BSD License

CC = g++
CC_OPTS = -c -pthread -I../src

EXE_NAME = test_chaudiere
LIB_NAME = ../src/chaudiere.so

OBJS = MockSocket.o \
TestAutoPointer.o \
TestByteBuffer.o \
TestCase.o \
TestIniReader.o \
TestInvalidKeyException.o \
TestKeyValuePairs.o \
TestKqueueServer.o \
TestOptionParser.o \
TestPthreadsMutex.o \
TestServerSocket.o \
TestSocket.o \
TestSocketRequest.o \
TestStrUtils.o \
TestStringTokenizer.o \
TestSuite.o \
TestSystemInfo.o \
TestSystemStats.o \
TestThread.o \
TestThreadInfo.o \
TestThreadPool.o \
TestThreadPoolQueue.o \
TestThreadPoolWorker.o \
Tests.o

all : $(EXE_NAME)

clean :
	rm -f *.o
	rm -f $(EXE_NAME)
	
$(EXE_NAME) : $(OBJS) $(ZLIB_OBJS)
	$(CC) $(OBJS) -o $(EXE_NAME) $(LIB_NAME) -lpthread -ldl

%.o : %.cpp
	$(CC) $(CC_OPTS) $< -o $@
