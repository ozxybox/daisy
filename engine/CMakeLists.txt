
set( SOURCES 
	engine.cpp

)


add_library( engine ${SOURCES} )
target_include_directories( engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../public )
target_link_libraries( engine glfw glad )
