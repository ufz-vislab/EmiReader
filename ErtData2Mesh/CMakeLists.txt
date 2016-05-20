add_executable(ErtData2Mesh ErtData2Mesh.cpp)
target_link_libraries(ErtData2Mesh
	logog
	BaseLib
	FileIO
	InSituLib
	${VTK_LIBRARIES}
)
ADD_VTK_DEPENDENCY(ErtData2Mesh)
set_target_properties(ErtData2Mesh PROPERTIES FOLDER Utilities)
