# Static by default (respects BUILD_SHARED_LIBS if a consumer sets it),
# same convention as poivre. The Makefile-built libchaudiere.so used by
# misere/tonnerre/chapeau's existing Makefile-based builds is a
# completely separate, unaffected build - this doesn't change that.
add_library(chaudiere
   DateTime.cpp
   DynamicLibrary.cpp
   EpollServer.cpp
   FileLogger.cpp
   IniReader.cpp
   InvalidKeyException.cpp
   KernelEventServer.cpp
   KeyValuePairs.cpp
   KqueueServer.cpp
   Logger.cpp
   NumberFormatException.cpp
   OSUtils.cpp
   OptionParser.cpp
   PthreadsConditionVariable.cpp
   PthreadsMutex.cpp
   PthreadsThread.cpp
   PthreadsThreadingFactory.cpp
   RequestHandler.cpp
   ServerSocket.cpp
   ServiceInfo.cpp
   Socket.cpp
   SocketRequest.cpp
   SocketServer.cpp
   StdConditionVariable.cpp
   StdLogger.cpp
   StdMutex.cpp
   StdThread.cpp
   StdThreadingFactory.cpp
   StrUtils.cpp
   StringTokenizer.cpp
   SystemInfo.cpp
   SystemStats.cpp
   Thread.cpp
   ThreadPool.cpp
   ThreadPoolDispatch.cpp
   ThreadPoolQueue.cpp
   ThreadPoolWorker.cpp
   ThreadingFactory.cpp
   Utils.cpp
)

target_compile_features(chaudiere PUBLIC cxx_std_20)
set_target_properties(chaudiere PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(chaudiere PRIVATE
   $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra>
)

# Headers live at this directory's root (no include/ subdirectory), so
# this is the one directory a consumer needs on its include path.
target_include_directories(chaudiere
   PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:include/chaudiere>
)

# SocketServer.cpp uses poivre's AutoPointer.h for RAII ownership of a
# RequestHandler* - implementation detail only (no chaudiere header
# includes it), so this is PRIVATE: consumers of the compiled chaudiere
# library don't need poivre at all. A plain include path rather than
# linking the poivre target, since the actual dependency is one header,
# not any of poivre's compiled code.
#
# SYSTEM (not plain PRIVATE): poivre ships its own BasicException.h/
# Runnable.h, same names as chaudiere's. Quote-includes always check the
# including file's own directory first, so today nothing in chaudiere/src
# can actually pick up poivre's copies by accident - but SYSTEM (-isystem,
# searched strictly after all -I dirs) makes that safe-by-construction
# rather than safe-by-coincidence-of-file-layout. See misere's
# src/CMakeLists.txt for a case (HttpException.cpp etc., living outside
# chaudiere's own directory) where the coincidence doesn't hold and this
# ordering is load-bearing, not just defensive.
target_include_directories(chaudiere SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../poivre)

find_package(Threads REQUIRED)
# CMAKE_DL_LIBS is chaudiere's own requirement (DynamicLibrary.cpp calls
# dlopen()/dlsym()), not just its tests' - PUBLIC so any consumer gets it
# automatically rather than having to remember -ldl themselves. Empty on
# glibc >= 2.34, where dl* moved into libc itself.
target_link_libraries(chaudiere PUBLIC Threads::Threads ${CMAKE_DL_LIBS})

include(GNUInstallDirs)

install(TARGETS chaudiere
   EXPORT chaudiereTargets
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

file(GLOB CHAUDIERE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES ${CHAUDIERE_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/chaudiere)
