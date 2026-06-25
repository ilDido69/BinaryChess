cmake_minimum_required(VERSION 3.16)

if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(BinaryChess)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3.0 REQUIRED COMPONENTS Graphics Window Audio)

include_directories(src)

set(SOURCES
	src/main.cpp
	src/gui.cpp
	src/moveGen.cpp
	src/search.cpp
 	src/gui.h
	src/moveGen.h
	src/utils.h
	src/lookUpTables.h
	src/search.h
)

add_executable(binaryChess ${SOURCES})

target_link_libraries(binaryChess SFML::Graphics SFML::Window SFML::Audio)

add_custom_command(TARGET binaryChess POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/assets"
	"$<TARGET_FILE_DIR:binaryChess>/assets"
)