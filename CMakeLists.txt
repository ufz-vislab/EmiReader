cmake_minimum_required(VERSION 2.8.12)
project(emi-reader)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++0x")

add_subdirectory(${CMAKE_SOURCE_DIR}/ogs)

include_directories(ogs)
include_directories(ogs/ThirdParty)

find_package(VTK 6.1 COMPONENTS vtkIOXML NO_MODULE QUIET)
include( ${VTK_USE_FILE} )

add_subdirectory(add-scalars)
