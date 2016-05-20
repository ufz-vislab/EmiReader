add_executable(addEmiDataToMesh addEmiDataToMesh.cpp)
target_link_libraries(addEmiDataToMesh
	logog
	BaseLib
	FileIO
	InSituLib
	${VTK_LIBRARIES}
)
ADD_VTK_DEPENDENCY(addEmiDataToMesh)
set_target_properties(addEmiDataToMesh PROPERTIES FOLDER Utilities)
