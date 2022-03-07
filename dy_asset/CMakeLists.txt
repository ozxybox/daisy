

set( DY_ALL_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../public )
set( DY_PUBLIC ${DY_ALL_PUBLIC}/asset )


set( SOURCES 
	${DY_PUBLIC}/dy_objimport.h
	dy_objimport.cpp
	dy_asset_png.cpp
)

add_library( dy_asset ${SOURCES} )
target_link_libraries( dy_asset dy_engine dy_util lodepng )
target_include_directories( dy_asset PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${DY_ALL_PUBLIC} )
target_include_directories( dy_asset PUBLIC ${DY_PUBLIC} )