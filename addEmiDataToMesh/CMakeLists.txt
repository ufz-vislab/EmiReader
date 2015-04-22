add_executable(addEmiDataToMesh addEmiDataToMesh.cpp)
target_link_libraries(addEmiDataToMesh
	logog
	BaseLib
	FileIO
	InSituLib
	${CATALYST_LIBRARIES}
)
ADD_CATALYST_DEPENDENCY(addEmiDataToMesh)
set_target_properties(addEmiDataToMesh PROPERTIES FOLDER Utilities)
