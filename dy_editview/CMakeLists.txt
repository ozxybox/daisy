set( DY_ALL_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../public )
set( DY_PUBLIC ${DY_ALL_PUBLIC}/editview )


set( SOURCES 
	dy_editview.cpp
	
	worldrender.h
	worldrender.cpp

	userpawn.h
	userpawn.cpp
)

add_library( dy_editview ${SOURCES} )
target_include_directories( dy_editview PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${DY_ALL_PUBLIC} )
target_include_directories( dy_editview PUBLIC ${DY_PUBLIC} )

target_link_libraries( dy_editview dy_engine dy_editsys dy_util dy_asset )
