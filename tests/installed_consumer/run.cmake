# Installed-consumer test, run by CTest as `cmake -P run.cmake` (see
# tests/CMakeLists.txt). It installs chaudiere and poivre into a scratch
# prefix and then checks, using nothing but that prefix, that:
#   - exactly the expected headers were installed: every header on
#     Linux/macOS (the installation is unchanged by the Windows work), only the
#     supported ones on Windows, so the unsupported APIs are absent;
#   - each installed header compiles on its own;
#   - the supported APIs link and work (main.cpp);
#   - on MSVC, StrUtils.h refuses to compile without /Zc:__cplusplus.
#
# Installed chaudiere is NOT self-contained, on any platform: its
# BasicException.h (and so NumberFormatException.h) includes
# "../poivre/BasicException.h", which relative to include/chaudiere/ is
# include/poivre/BasicException.h of the same prefix. A usable install is
# therefore all three of
#    <prefix>/include/chaudiere/   chaudiere's headers
#    <prefix>/<libdir>/            the chaudiere library
#    <prefix>/include/poivre/      poivre's headers
# This test builds that layout (poivre has to be installed separately, as it
# only installs itself when it is the top-level project) and checks the
# consumer against it. It does not test, and chaudiere does not support,
# installing chaudiere without poivre.
cmake_minimum_required(VERSION 3.16)

foreach(var CHAUDIERE_BINARY_DIR CHAUDIERE_SOURCE_DIR POIVRE_SOURCE_DIR WORK_DIR GENERATOR)
   if(NOT DEFINED ${var})
      message(FATAL_ERROR "run.cmake: ${var} is required")
   endif()
endforeach()

set(PREFIX ${WORK_DIR}/prefix)

# every command below runs through here so that a failure shows its output
function(run description)
   cmake_parse_arguments(RUN "EXPECT_FAILURE" "OUTPUT_VARIABLE" "COMMAND" ${ARGN})
   execute_process(
      COMMAND ${RUN_COMMAND}
      RESULT_VARIABLE rc
      OUTPUT_VARIABLE out
      ERROR_VARIABLE err
      ERROR_STRIP_TRAILING_WHITESPACE
   )
   if(RUN_OUTPUT_VARIABLE)
      set(${RUN_OUTPUT_VARIABLE} "${out}\n${err}" PARENT_SCOPE)
   endif()
   if(RUN_EXPECT_FAILURE)
      if(rc EQUAL 0)
         message(FATAL_ERROR "${description}: expected this to fail, but it succeeded\n${out}\n${err}")
      endif()
   elseif(NOT rc EQUAL 0)
      message(FATAL_ERROR "${description} failed (${rc})\n${out}\n${err}")
   endif()
endfunction()

# generator, platform, compiler and flags of the build under test, so the
# consumer is compiled and linked the way the library was (a sanitizer build
# needs its runtime at link time, for one)
set(GENERATOR_ARGS -G ${GENERATOR})
if(GENERATOR_PLATFORM)
   list(APPEND GENERATOR_ARGS -A ${GENERATOR_PLATFORM})
endif()
if(NOT GENERATOR MATCHES "Visual Studio")
   list(APPEND GENERATOR_ARGS -DCMAKE_CXX_COMPILER=${CXX_COMPILER})
   if(CONFIG)
      list(APPEND GENERATOR_ARGS -DCMAKE_BUILD_TYPE=${CONFIG})
   endif()
endif()
if(NOT PLATFORM_IS_WINDOWS)
   if(CXX_FLAGS)
      list(APPEND GENERATOR_ARGS "-DCMAKE_CXX_FLAGS=${CXX_FLAGS}")
   endif()
   if(EXE_LINKER_FLAGS)
      list(APPEND GENERATOR_ARGS "-DCMAKE_EXE_LINKER_FLAGS=${EXE_LINKER_FLAGS}")
   endif()
endif()
set(CONFIG_ARGS "")
if(CONFIG)
   set(CONFIG_ARGS --config ${CONFIG})
endif()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

# ---- install chaudiere and poivre (both are required) into one prefix --------
run("installing chaudiere"
   COMMAND ${CMAKE_COMMAND} --install ${CHAUDIERE_BINARY_DIR} --prefix ${PREFIX} ${CONFIG_ARGS})

run("configuring poivre"
   COMMAND ${CMAKE_COMMAND} -S ${POIVRE_SOURCE_DIR} -B ${WORK_DIR}/poivre-build ${GENERATOR_ARGS}
           -DPOIVRE_BUILD_TESTS=OFF -DPOIVRE_INSTALL=ON)
run("building poivre"
   COMMAND ${CMAKE_COMMAND} --build ${WORK_DIR}/poivre-build ${CONFIG_ARGS})
run("installing poivre"
   COMMAND ${CMAKE_COMMAND} --install ${WORK_DIR}/poivre-build --prefix ${PREFIX} ${CONFIG_ARGS})

# ---- the installed header set is exactly the expected one --------------------
file(GLOB installed RELATIVE ${PREFIX}/include/chaudiere ${PREFIX}/include/chaudiere/*)
list(SORT installed)

if(PLATFORM_IS_WINDOWS)
   # the contract for Windows, stated here independently of src/CMakeLists.txt:
   # the five supported components' headers plus what they include
   set(expected
      BasicException.h
      DateTime.h
      Logger.h
      NumberFormatException.h
      OSUtils.h
      StrUtils.h)
else()
   # everything, as before the Windows work
   file(GLOB expected RELATIVE ${CHAUDIERE_SOURCE_DIR}/src ${CHAUDIERE_SOURCE_DIR}/src/*.h)
endif()
list(SORT expected)

set(missing ${expected})
list(REMOVE_ITEM missing ${installed})
set(unexpected ${installed})
list(REMOVE_ITEM unexpected ${expected})
if(missing OR unexpected)
   message(FATAL_ERROR "installed headers differ from the expected set\n"
                       "  missing:    ${missing}\n  unexpected: ${unexpected}")
endif()
list(LENGTH installed header_count)
message(STATUS "installed headers match the expected set (${header_count})")

# unsupported APIs must really be absent on Windows, not just unlisted
if(PLATFORM_IS_WINDOWS)
   foreach(header Socket.h ServerSocket.h ThreadPool.h FileLogger.h StdLogger.h PthreadsMutex.h
                  DynamicLibrary.h SystemInfo.h SystemStats.h EpollServer.h KqueueServer.h)
      if(EXISTS ${PREFIX}/include/chaudiere/${header})
         message(FATAL_ERROR "${header} is installed, but its component is not supported on Windows")
      endif()
   endforeach()
endif()

# ---- configure the consumer and compile every header on its own --------------
set(consumer_dir ${WORK_DIR}/consumer)
# a list can't be handed through run() as one argument (its semicolons would split it)
string(REPLACE ";" "|" installed_arg "${installed}")
set(no_zc "")
if(PLATFORM_IS_MSVC)
   set(no_zc StrUtils.h)
endif()
run("configuring the consumer"
   COMMAND ${CMAKE_COMMAND} -S ${CMAKE_CURRENT_LIST_DIR} -B ${consumer_dir} ${GENERATOR_ARGS}
           -DCONSUMER_PREFIX=${PREFIX} "-DHEADERS=${installed_arg}" -DNO_ZC_HEADER=${no_zc})

set(failed "")
foreach(header IN LISTS installed)
   string(MAKE_C_IDENTIFIER ${header} id)
   execute_process(
      COMMAND ${CMAKE_COMMAND} --build ${consumer_dir} ${CONFIG_ARGS} --target hdr_${id}
      RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
   if(NOT rc EQUAL 0)
      list(APPEND failed ${header})
      message(WARNING "${header} does not compile on its own:\n${out}\n${err}")
   endif()
endforeach()
if(failed)
   message(FATAL_ERROR "installed headers that do not compile on their own: ${failed}")
endif()
message(STATUS "each of the ${header_count} installed headers compiles on its own")

# ---- the supported APIs link and work ----------------------------------------
run("building and running the consumer"
   COMMAND ${CMAKE_COMMAND} --build ${consumer_dir} ${CONFIG_ARGS} --target consumer
   OUTPUT_VARIABLE consumer_output)
if(NOT consumer_output MATCHES "CONSUMER-OK")
   message(FATAL_ERROR "the consumer did not report success\n${consumer_output}")
endif()
message(STATUS "the consumer linked and passed")

# ---- MSVC: StrUtils.h refuses to compile without /Zc:__cplusplus -------------
if(PLATFORM_IS_MSVC)
   run("compiling StrUtils.h without /Zc:__cplusplus"
      EXPECT_FAILURE
      COMMAND ${CMAKE_COMMAND} --build ${consumer_dir} ${CONFIG_ARGS} --target hdr_no_zc
      OUTPUT_VARIABLE no_zc_output)
   if(NOT no_zc_output MATCHES "requires /Zc:__cplusplus")
      message(FATAL_ERROR "StrUtils.h failed without /Zc:__cplusplus, but not with the expected error\n${no_zc_output}")
   endif()
   message(STATUS "StrUtils.h is refused without /Zc:__cplusplus, as intended")
endif()

message(STATUS "installed-consumer test passed")
