cmake_minimum_required(VERSION 3.16)

project(chaudiere
   DESCRIPTION "C++ core functionality: sockets, threading, configuration, logging"
   LANGUAGES CXX
)

# True only when chaudiere is configured directly, not pulled in via
# add_subdirectory() by a consumer (e.g. misere, tonnerre, chapeau).
# Controls whether chaudiere's own test suite is built by default - a
# consumer just wants the chaudiere library target, not chaudiere's
# ~40-file test suite and the poivre library it links against for that
# (the chaudiere library itself only needs one poivre header at compile
# time - see src/CMakeLists.txt - not the poivre library target).
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
   set(CHAUDIERE_IS_TOP_LEVEL TRUE)
else()
   set(CHAUDIERE_IS_TOP_LEVEL FALSE)
endif()

option(CHAUDIERE_BUILD_TESTS "Build chaudiere's own test suite" ${CHAUDIERE_IS_TOP_LEVEL})

add_subdirectory(src)

if(CHAUDIERE_BUILD_TESTS)
   add_subdirectory(poivre)
   enable_testing()
   add_subdirectory(tests)
endif()
