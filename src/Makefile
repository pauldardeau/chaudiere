# Copyright Paul Dardeau, SwampBits LLC 2014
# BSD License

CC = clang++
CC_OPTS = -c -std=c++11 -I./ThirdParty/zlib
ARCHIVE_CMD = ar
ARCHIVE_OPTS = rs

LIB_NAME = libchaudiere.a

ZLIB_OBJS = ./ThirdParty/zlib/adler32.o \
./ThirdParty/zlib/compress.o \
./ThirdParty/zlib/crc32.o \
./ThirdParty/zlib/deflate.o \
./ThirdParty/zlib/gzlib.o \
./ThirdParty/zlib/gzwrite.o \
./ThirdParty/zlib/inffast.o \
./ThirdParty/zlib/inflate.o \
./ThirdParty/zlib/inftrees.o \
./ThirdParty/zlib/trees.o \
./ThirdParty/zlib/zutil.o


OBJS = DateTime.o \
DynamicLibrary.o \
EpollServer.o \
IniReader.o \
InvalidKeyException.o \
KernelEventServer.o \
KeyValuePairs.o \
KqueueServer.o \
Logger.o \
PthreadsConditionVariable.o \
PthreadsMutex.o \
PthreadsThread.o \
RequestHandler.o \
ServerSocket.o \
ServiceInfo.o \
Socket.o \
SocketRequest.o \
SocketServer.o \
StdConditionVariable.o \
StdLogger.o \
StdMutex.o \
StdThread.o \
StrUtils.o \
StringTokenizer.o \
SystemInfo.o \
SystemStats.o \
Thread.o \
ThreadPool.o \
ThreadPoolDispatch.o \
ThreadPoolQueue.o \
ThreadPoolWorker.o \
ThreadingFactory.o

all : $(LIB_NAME)

clean :
	rm -f *.o
	rm -f $(LIB_NAME)
	cd ThirdParty/zlib && make clean
	
$(ZLIB_OBJS) :
	cd ThirdParty/zlib && make

$(LIB_NAME) : $(OBJS) $(ZLIB_OBJS)
	$(ARCHIVE_CMD) $(ARCHIVE_OPTS) $(LIB_NAME) $(OBJS) $(ZLIB_OBJS)

%.o : %.cpp
	$(CC) $(CC_OPTS) $< -o $@
