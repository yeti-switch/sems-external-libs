CMAKE_MINIMUM_REQUIRED(VERSION 3.6.2)

set(CMAKE_C_COMPILER clang-14 CACHE STRING "c compiler" FORCE)
set(CMAKE_CXX_COMPILER clang++-14 CACHE STRING "c++ compiler" FORCE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(sems-external-libs)

set(SEMS_THIRD_INCLUDE_PATH  /usr/include/sems/third)
set(SEMS_THIRD_LIB_PATH   /usr/lib/sems/third)

add_definitions(-D_THREAD_SAFE -D_REENTRANT)
add_compile_options(-Wall -Werror -fPIC -g -O2)
add_compile_options(-Wno-unknown-warning-option)

include(cmake/botan_bundled.cmake)
include(cmake/stun_bundled.cmake)
include(cmake/srtp_bundled.cmake)
include(cmake/curl_bundled.cmake)
include(cmake/spandsp_bundled.cmake)
include(cmake/confuse_bundled.cmake)
include(cmake/s3_bundled.cmake)

install(FILES sems_external_libs.cmake DESTINATION /usr/share/cmake/sems/)
