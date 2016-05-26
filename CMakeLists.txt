cmake_minimum_required(VERSION 2.8.12)
project(VisOgsTools)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  ${CMAKE_SOURCE_DIR}/ogs/scripts/cmake/cmake
  ${CMAKE_SOURCE_DIR}/ogs/scripts/cmake)
include(CompilerSetup)
include(ProjectSetup)

find_package( Qt4 )
add_subdirectory(${CMAKE_SOURCE_DIR}/ogs)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
include_directories(
	${CMAKE_SOURCE_DIR}/ogs
	${CMAKE_SOURCE_DIR}/ogs/ThirdParty
	${CMAKE_SOURCE_DIR}/ogs/ThirdParty/tclap/include
	${Boost_INCLUDE_DIRS}
	${CONAN_INCLUDE_DIRS}
)

add_subdirectory(addEmiDataToMesh)
add_subdirectory(addScalarArrayTimeSeries)
#add_subdirectory(EmiData2PolyData)
add_subdirectory(ErtData2Mesh)
if (QT4_FOUND)
	add_subdirectory(makeBuildings)
endif()
