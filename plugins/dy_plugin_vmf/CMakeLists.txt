

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../public )
add_executable( dy_plugin_vmf dy_plugin_vmf.cpp )
target_link_libraries( dy_plugin_vmf x9p dy_csg dy_util keyvalues )