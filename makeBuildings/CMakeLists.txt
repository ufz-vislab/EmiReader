# Find installed Qt4 libraries and headers

add_executable(makeBuildings makeBuildings.cpp)
target_link_libraries(makeBuildings
	logog
	BaseLib
	FileIO
	GeoLib
	${VTK_LIBRARIES}
)

if(TARGET Eigen)
	add_dependencies(makeBuildings Eigen)
endif()
if(TARGET Boost)
	add_dependencies(makeBuildings Boost)
endif()

ADD_VTK_DEPENDENCY(makeBuildings)
set_target_properties(makeBuildings PROPERTIES FOLDER Utilities)
