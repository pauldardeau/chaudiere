# Static by default (respects BUILD_SHARED_LIBS if a consumer sets it),
# same convention as poivre. The Makefile-built libchaudiere.so used by
# misere/tonnerre/chapeau's existing Makefile-based builds is a
# completely separate, unaffected build - this doesn't change that.

# The part of chaudiere that builds and is supported natively on Windows
# (MSVC): enough for chapeau. Everything here is portable C++ (or has an
# explicit Windows branch); see the platform notes in OSUtils.h for what
# OSUtils does and does not do on Windows.
set(CHAUDIERE_WINDOWS_SOURCES
   DateTime.cpp
   Logger.cpp
   NumberFormatException.cpp
   OSUtils.cpp
   StrUtils.cpp
)

# Everything else. Mostly POSIX-specific (pthreads, sockets, epoll/kqueue,
# dlopen, /proc- and sysctl-based system queries); the rest (e.g. IniReader,
# KeyValuePairs) is simply not part of the Windows build yet. Linux and macOS
# build both groups, which is the complete list this target always had.
set(CHAUDIERE_POSIX_SOURCES
   DynamicLibrary.cpp
   EpollServer.cpp
   FileLogger.cpp
   IniReader.cpp
   InvalidKeyException.cpp
   KernelEventServer.cpp
   KeyValuePairs.cpp
   KqueueServer.cpp
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

if(WIN32)
   set(CHAUDIERE_SOURCES ${CHAUDIERE_WINDOWS_SOURCES})
else()
   set(CHAUDIERE_SOURCES ${CHAUDIERE_WINDOWS_SOURCES} ${CHAUDIERE_POSIX_SOURCES})
endif()

add_library(chaudiere ${CHAUDIERE_SOURCES})

target_compile_features(chaudiere PUBLIC cxx_std_20)
set_target_properties(chaudiere PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
   # MSVC reports __cplusplus as 199711L whatever /std: says, unless this
   # is given, and StrUtils.h/.cpp pick their code (and, for the string_view
   # overloads, their declarations) from __cplusplus. Without it
   # StrUtils::parseLong() silently takes the pre-C++11 atol() path and
   # clamps instead of throwing. PUBLIC so a consumer compiles the same
   # header declarations the library was built with. CXX only: cl rejects
   # it for C sources.
   target_compile_options(chaudiere PUBLIC $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>)
   # MSVC is also true for clang-cl, where -Wall would mean -Weverything
   target_compile_options(chaudiere PRIVATE /W4)
else()
   target_compile_options(chaudiere PRIVATE
      $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra>
   )
endif()

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
# glibc >= 2.34, where dl* moved into libc itself (and on Windows).
target_link_libraries(chaudiere PUBLIC Threads::Threads ${CMAKE_DL_LIBS})

include(GNUInstallDirs)

install(TARGETS chaudiere
   EXPORT chaudiereTargets
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(WIN32)
   # Only the headers of the components built above, plus what they include
   # (NumberFormatException.h -> BasicException.h), so the unsupported APIs
   # are not installed and cannot be mistaken for available ones. The
   # installed-consumer test (tests/installed_consumer) compiles each of
   # these on its own, which is what keeps this list complete.
   # BasicException.h in turn includes "../poivre/BasicException.h": an
   # installed NumberFormatException.h needs poivre installed under the same
   # include/ directory (see the README).
   set(CHAUDIERE_HEADERS
      ${CMAKE_CURRENT_SOURCE_DIR}/BasicException.h
      ${CMAKE_CURRENT_SOURCE_DIR}/DateTime.h
      ${CMAKE_CURRENT_SOURCE_DIR}/Logger.h
      ${CMAKE_CURRENT_SOURCE_DIR}/NumberFormatException.h
      ${CMAKE_CURRENT_SOURCE_DIR}/OSUtils.h
      ${CMAKE_CURRENT_SOURCE_DIR}/StrUtils.h
   )
else()
   file(GLOB CHAUDIERE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
endif()
install(FILES ${CHAUDIERE_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/chaudiere)
