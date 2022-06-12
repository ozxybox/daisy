

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../public )
add_executable( dy_editds dy_editds.cpp )
target_link_libraries( dy_editds dy_editsys )