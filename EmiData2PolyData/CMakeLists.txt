add_executable(EmiData2PolyData EmiData2PolyData.cpp)
target_link_libraries(
  EmiData2PolyData
  ApplicationsFileIO
  GeoLib
  MeshLib
  MeshGeoToolsLib
  ${QT_LIBRARIES}
  tclap
  ogs_include)
set_target_properties(EmiData2PolyData PROPERTIES FOLDER Utilities)
