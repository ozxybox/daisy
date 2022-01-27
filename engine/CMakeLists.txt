
set( SOURCES 
	engine.cpp

	dy_math.h
	dy_math.cpp
)


add_library( engine ${SOURCES} )
target_include_directories( engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../public )
target_link_libraries( engine glfw glad )
