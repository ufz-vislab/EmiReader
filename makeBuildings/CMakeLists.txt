add_executable(makeBuildings makeBuildings.cpp)
target_link_libraries(
    makeBuildings
    BaseLib
    ApplicationsFileIO
    GeoLib
    MeshLib
    tclap
    ogs_include
)
set_target_properties(makeBuildings PROPERTIES FOLDER Utilities)
