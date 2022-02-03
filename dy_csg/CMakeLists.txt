
set( SOURCES 
	dy_csg.cpp
)


add_library( dy_csg ${SOURCES} )
target_include_directories( dy_csg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
target_include_directories( dy_csg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../public )
target_link_libraries( dy_csg dy_math )
