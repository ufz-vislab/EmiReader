cmake_minimum_required(VERSION 3.16)
project(VisOgsTools)

include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

CPMAddPackage(
    NAME tclap
    GITHUB_REPOSITORY ufz/tclap
    VERSION 1.2.4
    GIT_TAG 098dd0fe07a31618f3c2a9f8727bb01c8c5d61e2
    DOWNLOAD_ONLY YES
)
if(tclap_ADDED)
    add_library(tclap INTERFACE IMPORTED)
    target_include_directories(
        tclap SYSTEM INTERFACE ${tclap_SOURCE_DIR}/include
    )
endif()

CPMAddPackage(
    NAME ogs
    GIT_REPOSITORY https://gitlab.opengeosys.org/ogs/ogs.git
    GIT_TAG 98aeeb379ac87cc88fb71d111bcb946e2db4c16f
    VERSION 6.4.0-dev
    OPTIONS "OGS_BUILD_GUI ON" "OGS_BUILD_CLI OFF" "OGS_BUILD_TESTING OFF"
    EXCLUDE_FROM_ALL
)
if(ogs_ADDED)
    add_library(ogs_include INTERFACE IMPORTED)
    target_include_directories(ogs_include SYSTEM INTERFACE ${ogs_SOURCE_DIR}/)
endif()

# find_package(Qt4)

# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake) include_directories(
# ${CMAKE_SOURCE_DIR}/ogs ${CMAKE_SOURCE_DIR}/ogs/ThirdParty
# ${CMAKE_SOURCE_DIR}/ogs/ThirdParty/tclap/include ${Boost_INCLUDE_DIRS}
# ${CONAN_INCLUDE_DIRS} )

add_subdirectory(addEmiDataToMesh)
add_subdirectory(addScalarArrayTimeSeries)
add_subdirectory(EmiData2PolyData)
# add_subdirectory(ErtData2Mesh) if (QT4_FOUND) add_subdirectory(makeBuildings)
# endif()
