add_executable(ErtData2Mesh ErtData2Mesh.cpp)
target_link_libraries(
    ErtData2Mesh BaseLib ApplicationsFileIO MeshLib tclap ogs_include
)
set_target_properties(ErtData2Mesh PROPERTIES FOLDER Utilities)
