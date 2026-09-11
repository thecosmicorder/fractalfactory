cmake_minimum_required(VERSION 3.20)
project(FractalFactory VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)

if(NOT WIN32)
  message(FATAL_ERROR "Fractal Factory V1 currently targets Windows x64.")
endif()

find_package(GLEW REQUIRED)
set(FFGL_BUILD_EXAMPLE_PLUGINS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/ffgl EXCLUDE_FROM_ALL)

add_library(FractalFactory MODULE
  src/FFGLFractalFactory.cpp
  src/FFGLFractalFactory.h
  src/FractalFactoryShared.h
)
target_include_directories(FractalFactory PRIVATE external/ffgl/source/lib src)
target_link_libraries(FractalFactory PRIVATE ffgl::sdk GLEW::GLEW opengl32)
target_compile_definitions(FractalFactory PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
set_target_properties(FractalFactory PROPERTIES PREFIX "" OUTPUT_NAME "FractalFactory")

add_executable(FractalFactoryControl WIN32
  controller/FractalFactoryControl.cpp
  src/FractalFactoryShared.h
)
target_include_directories(FractalFactoryControl PRIVATE src)
target_link_libraries(FractalFactoryControl PRIVATE comctl32 user32 gdi32)
target_compile_definitions(FractalFactoryControl PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

install(TARGETS FractalFactory FractalFactoryControl RUNTIME DESTINATION . LIBRARY DESTINATION .)
