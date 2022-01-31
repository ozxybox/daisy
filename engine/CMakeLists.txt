
set( SOURCES 
	dy_engine.cpp
	
	dy_math.h
	dy_math.cpp
	
	dy_shader.h
	dy_shader.cpp
	
	dy_vertex.h
	dy_vertex.cpp

	dy_render.h
	dy_render.cpp
)


add_library( engine ${SOURCES} )
target_include_directories( engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../public )
target_link_libraries( engine glfw glad )
