

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../public )
add_executable( launcher launcher.cpp server.cpp )
target_link_libraries( launcher x9p dy_engine dy_csg dy_util )