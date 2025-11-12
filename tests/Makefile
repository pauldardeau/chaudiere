# Copyright Paul Dardeau, SwampBits LLC 2014
# BSD License

CC = c++
CC_OPTS = -c -g -std=c++20 -pthread -I../src -I../poivre
#CC_OPTS = -c -std=c++20 -pthread -I../src -I../poivre

EXE_NAME = test_chaudiere
LIB_NAMES = ../src/libchaudiere.so ../poivre/poivre.so

OBJS = MockSocket.o \
TestAutoPointer.o \
TestByteBuffer.o \
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
	$(CC) $(OBJS) -o $(EXE_NAME) $(LIB_NAMES) -lpthread -ldl

%.o : %.cpp
	$(CC) $(CC_OPTS) $< -o $@
