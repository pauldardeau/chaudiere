# A consumer of an *installed* chaudiere, configured by run.cmake. It knows
# nothing about the chaudiere source tree: only the install prefix.
#
# Inputs: CONSUMER_PREFIX (the install prefix), HEADERS (the installed headers, as file
# names separated by |), and on MSVC NO_ZC_HEADER (a header to compile without
# /Zc:__cplusplus, which must fail).
cmake_minimum_required(VERSION 3.16)
project(chaudiere_installed_consumer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

string(REPLACE "|" ";" HEADERS "${HEADERS}")

set(INCLUDE_DIR ${CONSUMER_PREFIX}/include/chaudiere)

# the installed library carries no exported target, so a consumer has to give
# the compiler what the in-tree target adds by itself
if(MSVC)
   set(CONSUMER_FLAGS /Zc:__cplusplus /W4)
else()
   set(CONSUMER_FLAGS "")
endif()

# one translation unit per installed header that includes nothing else, so that
# a header relying on something it doesn't include itself fails here
foreach(header IN LISTS HEADERS)
   string(MAKE_C_IDENTIFIER ${header} id)
   file(WRITE ${CMAKE_BINARY_DIR}/hdr/${id}.cpp "#include <${header}>\n")
   add_library(hdr_${id} OBJECT EXCLUDE_FROM_ALL ${CMAKE_BINARY_DIR}/hdr/${id}.cpp)
   target_include_directories(hdr_${id} PRIVATE ${INCLUDE_DIR})
   target_compile_options(hdr_${id} PRIVATE ${CONSUMER_FLAGS})
endforeach()

# MSVC only: StrUtils.h without /Zc:__cplusplus must refuse to compile
if(MSVC AND NO_ZC_HEADER)
   file(WRITE ${CMAKE_BINARY_DIR}/hdr/no_zc.cpp "#include <${NO_ZC_HEADER}>\n")
   add_library(hdr_no_zc OBJECT EXCLUDE_FROM_ALL ${CMAKE_BINARY_DIR}/hdr/no_zc.cpp)
   target_include_directories(hdr_no_zc PRIVATE ${INCLUDE_DIR})
   target_compile_options(hdr_no_zc PRIVATE /W4)
endif()

find_library(CHAUDIERE_LIB NAMES chaudiere PATHS ${CONSUMER_PREFIX} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
if(NOT CHAUDIERE_LIB)
   message(FATAL_ERROR "no chaudiere library under ${CONSUMER_PREFIX}/lib")
endif()

find_package(Threads REQUIRED)

add_executable(consumer EXCLUDE_FROM_ALL main.cpp)
target_include_directories(consumer PRIVATE ${INCLUDE_DIR})
target_compile_options(consumer PRIVATE ${CONSUMER_FLAGS})
target_link_libraries(consumer PRIVATE ${CHAUDIERE_LIB} Threads::Threads ${CMAKE_DL_LIBS})
# run it as part of building it, from a scratch working directory
add_custom_command(TARGET consumer POST_BUILD
   COMMAND consumer
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
   COMMENT "running the installed-consumer program"
   VERBATIM
)
