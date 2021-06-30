add_executable(addEmiDataToMesh addEmiDataToMesh.cpp)
target_link_libraries(addEmiDataToMesh ApplicationsFileIO BaseLib MeshLib tclap
                      ogs_include)
set_target_properties(addEmiDataToMesh PROPERTIES FOLDER Utilities)
