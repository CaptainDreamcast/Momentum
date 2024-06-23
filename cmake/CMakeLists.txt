cmake_minimum_required(VERSION 3.25) 
project(Momentum CXX)               
set(CMAKE_CXX_STANDARD 17)            
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include("${CMAKE_SOURCE_DIR}/../../addons/prism/cmake/prism.cmake" REQUIRED)

# Add source/header files to project
file(GLOB_RECURSE SOURCES ../*.cpp SOURCES ../*.h SOURCES ../*.rc)
list(FILTER SOURCES EXCLUDE REGEX ".*web.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/build/.*")

add_link_options(/NODEFAULTLIB:libcmt.lib)
add_link_options(/IGNORE:4099,4286,4098)
add_link_options(/OPT:REF)
add_link_options(/OPT:ICF)

# Define exe
add_executable(Momentum ${SOURCES})

# Link with libraries
target_link_libraries(Momentum prism)
target_link_libraries(Momentum SDL2)
target_link_libraries(Momentum SDL2main)
target_link_libraries(Momentum SDL2_image)
target_link_libraries(Momentum SDL2_mixer)
target_link_libraries(Momentum SDL2_ttf)
target_link_libraries(Momentum enet)
target_link_libraries(Momentum freetype)
target_link_libraries(Momentum glew)
target_link_libraries(Momentum curl)
target_link_libraries(Momentum libjpeg)
target_link_libraries(Momentum libpng)
target_link_libraries(Momentum libwebp)
target_link_libraries(Momentum zstd)
target_link_libraries(Momentum zlib)
target_link_libraries(Momentum crypt32)
target_link_libraries(Momentum wldap32)
target_link_libraries(Momentum ws2_32)
target_link_libraries(Momentum winmm)
target_link_libraries(Momentum OpenGL32)
target_link_libraries(Momentum version)

# Define preprocessor definitions
target_compile_definitions(Momentum PUBLIC UNICODE)
target_compile_definitions(Momentum PUBLIC _UNICODE)

set_property(TARGET Momentum PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_compile_options(Momentum PRIVATE /Gy)

set_target_properties(Momentum PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../assets)
set_target_properties(Momentum PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../assets)

# Copy over DLLs
file(GLOB DLLS "${CMAKE_SOURCE_DIR}/../../addons/prism/windows/vs17/DLL/*.dll")
foreach(DLL ${DLLS})
  add_custom_command(TARGET Momentum POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DLL}
    $<TARGET_FILE_DIR:Momentum>)
endforeach()

# Optimize for release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(Momentum PRIVATE /O2)
endif()
