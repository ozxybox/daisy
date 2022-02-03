
set( SOURCES 
	dy_engine.cpp
	
	dy_shader.h
	dy_shader.cpp
	
	dy_vertex.h
	dy_vertex.cpp
	
	dy_render.h
	dy_render.cpp

	dy_texture.h
	dy_texture.cpp
)


add_library( engine ${SOURCES} )
target_include_directories( engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
target_include_directories( engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../public )
target_link_libraries( engine dy_math glfw glad )
