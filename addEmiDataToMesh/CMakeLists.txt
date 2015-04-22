add_executable(addScalarArray addScalarArray.cpp)
target_link_libraries(addScalarArray
	logog
	BaseLib
	FileIO
	InSituLib
	${CATALYST_LIBRARIES}
)
ADD_CATALYST_DEPENDENCY(addScalarArray)
set_target_properties(addScalarArray PROPERTIES FOLDER Utilities)
