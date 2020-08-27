cmake_minimum_required(VERSION 3.13)

project(RTXX VERSION 0.2.1)

option(RTXX_USE_POSIX "Use POSIX threads interface" TRUE)
option(RTXX_USE_ALCHEMY "Use Alchemy API" FALSE)
option(RTXX_USE_RTDM "Use RTDM skin" FALSE)

set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(Threads)
enable_testing()

include(GNUInstallDirs)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(doc)
add_subdirectory(cmake)

set(CPACK_GENERATOR STGZ TGZ)
include(CPack)
