add_executable(addScalarArrayTimeSeries addScalarArrayTimeSeries.cpp)
target_link_libraries(addScalarArrayTimeSeries
	logog
	BaseLib
	FileIO
	InSituLib
	${CATALYST_LIBRARIES}
)
ADD_CATALYST_DEPENDENCY(addScalarArrayTimeSeries)
set_target_properties(addScalarArrayTimeSeries PROPERTIES FOLDER Utilities)
