add_executable(ErtData2Mesh ErtData2Mesh.cpp)
target_link_libraries(ErtData2Mesh
	logog
	BaseLib
	FileIO
	InSituLib
	${CATALYST_LIBRARIES}
)
ADD_CATALYST_DEPENDENCY(ErtData2Mesh)
set_target_properties(ErtData2Mesh PROPERTIES FOLDER Utilities)
