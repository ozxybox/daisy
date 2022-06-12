

set( DY_ALL_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../public )
set( DY_PUBLIC ${DY_ALL_PUBLIC}/engine )


set( SOURCES 
	${DY_PUBLIC}/dy_engine.h
	dy_engine.cpp
	
	${DY_PUBLIC}/dy_shader.h
	dy_shader.cpp
	
	${DY_PUBLIC}/dy_vertex.h
	dy_vertex.cpp
	
	${DY_PUBLIC}/dy_render.h
	dy_render.cpp

	${DY_PUBLIC}/dy_texture.h
	dy_texture.cpp

	${DY_PUBLIC}/dy_doodle.h
	dy_doodle.cpp

	dy_primitives.h
	dy_primitives.cpp

	
	${DY_PUBLIC}/dy_debugtext.h
	dy_debugtext.cpp

)


add_library( dy_engine ${SOURCES} )
target_include_directories( dy_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${DY_ALL_PUBLIC} )
target_include_directories( dy_engine PUBLIC ${DY_PUBLIC} )
target_link_libraries( dy_engine dy_util dy_asset dy_csg glfw glad )
