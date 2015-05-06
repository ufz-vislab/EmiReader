cmake_minimum_required(VERSION 2.8.12)
project(VisOgsTools)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  ${CMAKE_SOURCE_DIR}/ogs/scripts/cmake/cmake
  ${CMAKE_SOURCE_DIR}/ogs/scripts/cmake)
include(CompilerSetup)
include(ProjectSetup)

add_subdirectory(${CMAKE_SOURCE_DIR}/ogs)

include_directories(ogs)
include_directories(ogs/ThirdParty)
include_directories(${Boost_INCLUDE_DIRS})

find_package(VTK 6.1 COMPONENTS vtkIOXML NO_MODULE QUIET)
include( ${VTK_USE_FILE} )

add_subdirectory(addEmiDataToMesh)
add_subdirectory(addScalarArrayTimeSeries)
add_subdirectory(ErtData2Mesh)
