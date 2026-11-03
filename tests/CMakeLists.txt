if(WIN32)
   # The Windows (MSVC) build of chaudiere is deliberately limited to five
   # components (see src/CMakeLists.txt), so this is a smaller test executable
   # with its own driver, TestsWindows.cpp. Tests.cpp, which lists every suite,
   # is not used on Windows.
   #
   # Suites of the five Windows components:
   #    TestDateTime, TestNumberFormatException, TestOSUtils, TestStrUtils
   # (Logger has no suite of its own: the suites that exercise it,
   # TestFileLogger and TestStdLogger, are POSIX-only. The installed-consumer
   # test below does exercise it on Windows.)
   #
   # Suites of header-only classes, which need no library at all:
   #    TestAutoPointer, TestByteBuffer, TestCharBuffer
   #
   # Suites of components that are NOT part of the Windows library. They are
   # kept running on Windows because they pass there, so their sources are
   # compiled directly into this executable. That makes them tested, not
   # supported: they are not part of the chaudiere library on Windows.
   #    TestIniReader, TestInvalidKeyException, TestKeyValuePairs,
   #    TestOptionParser, TestServiceInfo, TestStringTokenizer, TestUtils
   #
   # Not built on Windows at all, because the component under test is not part
   # of the Windows library: POSIX-only (sockets, epoll/kqueue, dlopen,
   # pthreads, system information) or not ported (the threading, thread pool
   # and Std* classes, FileLogger/StdLogger):
   #    MockSocket, TestDynamicLibrary, TestEpollServer, TestFileLogger,
   #    TestKqueueServer, TestMutexLock, TestPthreadsConditionVariable,
   #    TestPthreadsMutex, TestPthreadsThreadingFactory, TestRequestHandler,
   #    TestServerSocket, TestSocket, TestSocketRequest, TestSocketServer,
   #    TestStdConditionVariable, TestStdLogger, TestStdMutex, TestStdThread,
   #    TestStdThreadingFactory, TestSystemInfo, TestSystemStats, TestThread,
   #    TestThreadInfo, TestThreadPool, TestThreadPoolQueue,
   #    TestThreadPoolWorker, TestThreadingFactory
   set(CHAUDIERE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src)
   add_executable(test_chaudiere
      TestsWindows.cpp
      TestAutoPointer.cpp
      TestByteBuffer.cpp
      TestCharBuffer.cpp
      TestDateTime.cpp
      TestIniReader.cpp
      TestInvalidKeyException.cpp
      TestKeyValuePairs.cpp
      TestNumberFormatException.cpp
      TestOptionParser.cpp
      TestOSUtils.cpp
      TestServiceInfo.cpp
      TestStrUtils.cpp
      TestStringTokenizer.cpp
      TestUtils.cpp
      ${CHAUDIERE_SRC}/IniReader.cpp
      ${CHAUDIERE_SRC}/InvalidKeyException.cpp
      ${CHAUDIERE_SRC}/KeyValuePairs.cpp
      ${CHAUDIERE_SRC}/OptionParser.cpp
      ${CHAUDIERE_SRC}/ServiceInfo.cpp
      ${CHAUDIERE_SRC}/StringTokenizer.cpp
      ${CHAUDIERE_SRC}/Utils.cpp
   )
   # MSVC is also true for clang-cl, where -Wall would mean -Weverything
   target_compile_options(test_chaudiere PRIVATE /W4)
else()
   add_executable(test_chaudiere
      MockSocket.cpp
      TestAutoPointer.cpp
      TestByteBuffer.cpp
      TestCharBuffer.cpp
      TestDateTime.cpp
      TestDynamicLibrary.cpp
      TestEpollServer.cpp
      TestFileLogger.cpp
      TestIniReader.cpp
      TestInvalidKeyException.cpp
      TestKeyValuePairs.cpp
      TestKqueueServer.cpp
      TestMutexLock.cpp
      TestNumberFormatException.cpp
      TestOptionParser.cpp
      TestOSUtils.cpp
      TestPthreadsConditionVariable.cpp
      TestPthreadsMutex.cpp
      TestPthreadsThreadingFactory.cpp
      TestRequestHandler.cpp
      TestServerSocket.cpp
      TestServiceInfo.cpp
      TestSocket.cpp
      TestSocketRequest.cpp
      TestSocketServer.cpp
      TestStdConditionVariable.cpp
      TestStdLogger.cpp
      TestStdMutex.cpp
      TestStdThread.cpp
      TestStdThreadingFactory.cpp
      TestStrUtils.cpp
      TestStringTokenizer.cpp
      TestSystemInfo.cpp
      TestSystemStats.cpp
      TestThread.cpp
      TestThreadInfo.cpp
      TestThreadPool.cpp
      TestThreadPoolQueue.cpp
      TestThreadPoolWorker.cpp
      TestThreadingFactory.cpp
      TestUtils.cpp
      Tests.cpp
   )
endif()

# chaudiere and poivre both propagate their own C++20 requirement and
# include directories via target_link_libraries - nothing extra needed
# here for either.
target_link_libraries(test_chaudiere PRIVATE chaudiere poivre)

# matches how the Makefile-based build always runs it - some tests
# (e.g. FileLogger) touch relative-path files expecting tests/ as the
# working directory.
add_test(
   NAME test_chaudiere
   COMMAND test_chaudiere
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
