
cmake_minimum_required (VERSION 3.0)



set( SOURCES 
	main.cpp
	
	vpk_format.h

	vpk_fs.h
	vpk_fs.cpp
)


add_executable( dy_plugin_vpk ${SOURCES} )
target_include_directories( dy_plugin_vpk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
target_link_libraries( dy_plugin_vpk x9p dy_util )

