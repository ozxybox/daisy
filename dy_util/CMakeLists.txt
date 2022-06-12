

set( DY_ALL_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../public )
set( DY_PUBLIC ${DY_ALL_PUBLIC}/util )


set( SOURCES 
	${DY_PUBLIC}/dy_math.h
	dy_math.cpp

	${DY_PUBLIC}/dy_ustack.h


	${DY_PUBLIC}/dy_snooze.h
	dy_snooze.cpp
)

add_library( dy_util ${SOURCES} )
target_include_directories( dy_util PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${DY_ALL_PUBLIC} )
target_include_directories( dy_util PUBLIC ${DY_PUBLIC} )


add_executable( test_dy_util test_dy_util.cpp )
target_link_libraries( test_dy_util dy_util )

add_test(NAME test_dy_ustack COMMAND test_dy_util 0)
add_test(NAME test_dy_mat3_solve COMMAND test_dy_util 1)