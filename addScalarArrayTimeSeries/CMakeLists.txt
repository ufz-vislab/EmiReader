add_executable(addScalarArrayTimeSeries addScalarArrayTimeSeries.cpp)
target_link_libraries(addScalarArrayTimeSeries
	logog
	BaseLib
	FileIO
	InSituLib
	${VTK_LIBRARIES}
)
ADD_VTK_DEPENDENCY(addScalarArrayTimeSeries)
set_target_properties(addScalarArrayTimeSeries PROPERTIES FOLDER Utilities)
