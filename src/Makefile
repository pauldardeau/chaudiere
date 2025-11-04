# Copyright Paul Dardeau, SwampBits LLC 2014
# BSD License

CC = c++
CC_OPTS = -c -Wall -std=c++20 -fPIC -O2 -pthread -I/usr/local/include
ARCHIVE_CMD = ar
ARCHIVE_OPTS = rs

LIB_NAME = libchaudiere.so

OBJS = DateTime.o \
DynamicLibrary.o \
EpollServer.o \
FileLogger.o \
IniReader.o \
InvalidKeyException.o \
KernelEventServer.o \
KeyValuePairs.o \
KqueueServer.o \
Logger.o \
NumberFormatException.o \
OSUtils.o \
OptionParser.o \
PthreadsConditionVariable.o \
PthreadsMutex.o \
PthreadsThread.o \
RequestHandler.o \
ServerSocket.o \
ServiceInfo.o \
Socket.o \
SocketRequest.o \
SocketServer.o \
StdLogger.o \
StrUtils.o \
StringTokenizer.o \
SystemInfo.o \
SystemStats.o \
Thread.o \
ThreadPool.o \
ThreadPoolDispatch.o \
ThreadPoolQueue.o \
ThreadPoolWorker.o \
ThreadingFactory.o \
PthreadsThreadingFactory.o \
Utils.o

all : $(LIB_NAME)

clean :
	rm -f *.o
	rm -f $(LIB_NAME)
	
$(LIB_NAME) : $(OBJS)
	$(CC) -shared -fPIC $(OBJS) -o $(LIB_NAME)

%.o : %.cpp
	$(CC) $(CC_OPTS) $< -o $@
