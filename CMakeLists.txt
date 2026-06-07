cmake_minimum_required(VERSION 3.16)

if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(BinaryChess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3.0 REQUIRED COMPONENTS Graphics Window Audio)

include_directories(src)

set(SOURCES
	src/main.cpp
	src/game.cpp
	src/moveGenerator.cpp
 	src/game.h
	src/moveGenerator.h
)

add_executable(binaryChess ${SOURCES})

target_link_libraries(binaryChess SFML::Graphics SFML::Window SFML::Audio)

add_custom_command(TARGET binaryChess POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/assets"
	"${CMAKE_BINARY_DIR}/$<CONFIG>/assets"
	COMMENT "Copying assets to build directory..."
)