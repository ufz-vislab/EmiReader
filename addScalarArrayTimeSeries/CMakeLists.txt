add_executable(addScalarArrayTimeSeries addScalarArrayTimeSeries.cpp)
target_link_libraries(addScalarArrayTimeSeries ApplicationsFileIO BaseLib
                      MeshLib tclap ogs_include)
set_target_properties(addScalarArrayTimeSeries PROPERTIES FOLDER Utilities)
