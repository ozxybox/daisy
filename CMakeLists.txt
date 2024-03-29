cmake_minimum_required( VERSION 3.0 )

# Check for an in source root build
STRING(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR} insourceroot )
if(insourceroot)
message( FATAL_ERROR "\nPlease change your build directory to something like \"build\" and not the source root!\ncmake . -B build" )
endif()

# Set outputs to one spot
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/out )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/out )

enable_testing()

project( Daisy )

# Configure GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Thirdparty libraries
add_subdirectory( thirdparty/glad )
add_subdirectory( thirdparty/glfw )
add_subdirectory( thirdparty/x9p )
add_subdirectory( thirdparty/keyvalues )
include( thirdparty/lodepng.cmake )

add_subdirectory( dy_util )
add_subdirectory( dy_csg )
add_subdirectory( dy_engine )
add_subdirectory( dy_asset )
add_subdirectory( dy_editds )
add_subdirectory( dy_editsys )
add_subdirectory( dy_editview )
add_subdirectory( launcher )
add_subdirectory( plugins )
