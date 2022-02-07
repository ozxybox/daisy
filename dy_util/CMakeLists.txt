

set( DY_ALL_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../public )
set( DY_PUBLIC ${DY_ALL_PUBLIC}/util )


set( SOURCES 
	${DY_PUBLIC}/dy_math.h
	dy_math.cpp

	${DY_PUBLIC}/dy_ustack.h
)

add_library( dy_util ${SOURCES} )
target_include_directories( dy_util PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${DY_ALL_PUBLIC} )
target_include_directories( dy_util PUBLIC ${DY_PUBLIC} )