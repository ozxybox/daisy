

set( DY_ALL_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../public )
set( DY_PUBLIC ${DY_ALL_PUBLIC}/csg )

set( SOURCES 
	${DY_PUBLIC}/dy_csg.h
	dy_csg.cpp

	${DY_PUBLIC}/dy_halfedge.h
	dy_halfedge.cpp
	
	${DY_PUBLIC}/dy_brush.h
	dy_brush.cpp
)


add_library( dy_csg ${SOURCES} )
target_include_directories( dy_csg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${DY_ALL_PUBLIC} )
target_include_directories( dy_csg PUBLIC ${DY_PUBLIC} )
target_link_libraries( dy_csg dy_util )
