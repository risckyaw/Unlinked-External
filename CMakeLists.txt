cmake_minimum_required( VERSION 3.20 )

project( Unlinked LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

if( NOT WIN32 )
    message( FATAL_ERROR "Unlinked builds on Windows." )
endif( )

set( UR_BUILD_DEMO OFF CACHE BOOL "" FORCE )
set( UR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/custom-framework" )
set( FONT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/fonts" )

if( NOT EXISTS "${UR_ROOT}/CMakeLists.txt" )
    message( FATAL_ERROR "Missing ${UR_ROOT}. The source tree must include third_party/custom-framework." )
endif( )

add_subdirectory( "${UR_ROOT}" "${CMAKE_BINARY_DIR}/ur" )

add_executable( Unlinked WIN32 src/Main.cpp )
target_link_libraries( Unlinked PRIVATE ur winhttp )
target_include_directories( Unlinked PRIVATE src )

if( MSVC )
    target_compile_options( Unlinked PRIVATE /W4 /utf-8 /EHsc )
endif( )

target_precompile_headers( Unlinked PRIVATE
    <Windows.h>
    <cmath>
    <cstdint>
    <cstdio>
    <cstring>
    <string>
    <vector>
    <memory>
    <chrono>
)

set_target_properties( Unlinked PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command( TARGET Unlinked POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${UR_ROOT}/assets"
        "$<TARGET_FILE_DIR:Unlinked>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/Unlinked.webp"
        "$<TARGET_FILE_DIR:Unlinked>/assets/Unlinked.webp"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:Unlinked>/assets/icons/fontawesome"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FONT_DIR}/fa-solid-900.woff2"
        "$<TARGET_FILE_DIR:Unlinked>/assets/icons/fontawesome/fa-solid-900.woff2"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FONT_DIR}/fa-regular-400.woff2"
        "$<TARGET_FILE_DIR:Unlinked>/assets/icons/fontawesome/fa-regular-400.woff2"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FONT_DIR}/fa-brands-400.woff2"
        "$<TARGET_FILE_DIR:Unlinked>/assets/icons/fontawesome/fa-brands-400.woff2"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:Unlinked>/assets/fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FONT_DIR}/Poppins-Regular.ttf"
        "$<TARGET_FILE_DIR:Unlinked>/assets/fonts/Poppins-Regular.ttf"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FONT_DIR}/Poppins-Medium.ttf"
        "$<TARGET_FILE_DIR:Unlinked>/assets/fonts/Poppins-Medium.ttf"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FONT_DIR}/Poppins-SemiBold.ttf"
        "$<TARGET_FILE_DIR:Unlinked>/assets/fonts/Poppins-SemiBold.ttf"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FONT_DIR}/Inter-Regular.ttf"
        "$<TARGET_FILE_DIR:Unlinked>/assets/fonts/Inter-Regular.ttf"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FONT_DIR}/Inter-Medium.ttf"
        "$<TARGET_FILE_DIR:Unlinked>/assets/fonts/Inter-Medium.ttf"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FONT_DIR}/Inter-SemiBold.ttf"
        "$<TARGET_FILE_DIR:Unlinked>/assets/fonts/Inter-SemiBold.ttf"
)

enable_testing( )

add_executable( UnlinkedTests
    tests/test_main.cpp
    tests/test_math.cpp
    tests/test_store.cpp
    tests/test_offsets.cpp
    tests/test_handle.cpp
    tests/test_geometry.cpp
    tests/test_weather.cpp
    tests/test_move.cpp
    tests/test_sense.cpp
)
target_link_libraries( UnlinkedTests PRIVATE winhttp )
target_include_directories( UnlinkedTests PRIVATE src tests )
if( MSVC )
    target_compile_options( UnlinkedTests PRIVATE /W4 /utf-8 /EHsc )
endif( )

target_precompile_headers( UnlinkedTests PRIVATE
    <cmath>
    <cstdint>
    <cstdio>
    <cstdlib>
    <cstring>
    <string>
    <vector>
    <Windows.h>
)

add_test( NAME UnlinkedTests COMMAND UnlinkedTests )


