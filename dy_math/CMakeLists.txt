
add_library( dy_math dy_math.cpp )
target_include_directories( dy_math PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
target_include_directories( dy_math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../public )