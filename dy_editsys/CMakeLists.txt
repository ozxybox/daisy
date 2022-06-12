set( DY_ALL_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../public )
set( DY_PUBLIC ${DY_ALL_PUBLIC}/editsys )


set( SOURCES 

# Client
	client/dy_editclient.cpp
	client/dy_editclient.cpp

# Server
	server/dy_editserver.cpp

# Shared

	world.cpp
	world.h
	
	dy_editsys.h
	dy_editsys.cpp

	userinfo.h
	userinfo.cpp

	dy_netdb.h
	dy_netdb.cpp

	dy_ot.h
	dy_ot.cpp
)

add_library( dy_editsys ${SOURCES} )
target_include_directories( dy_editsys PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${DY_ALL_PUBLIC} )
#target_include_directories( dy_editsys PUBLIC ${DY_PUBLIC} )

# FIXME: too much sharing!
target_include_directories( dy_editsys PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${DY_PUBLIC} )


target_link_libraries( dy_editsys dy_util dy_csg x9p )
