# Copyright Paul Dardeau, SwampBits LLC 2014
# BSD License

CC = c++
CC_OPTS = -c -Wall -fPIC -O2 -pthread -I./ThirdParty/zlib -I/usr/local/include
ARCHIVE_CMD = ar
ARCHIVE_OPTS = rs

LIB_NAME = chaudiere.so

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
	
$(ZLIB_OBJS) :
	cd ThirdParty/zlib && make

$(LIB_NAME) : $(OBJS)
	$(CC) -shared -fPIC $(OBJS) -o $(LIB_NAME)

%.o : %.cpp
	$(CC) $(CC_OPTS) $< -o $@
