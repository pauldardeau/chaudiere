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
