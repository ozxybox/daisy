

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../public )
add_executable( launcher launcher.cpp )
target_link_libraries( launcher engine )