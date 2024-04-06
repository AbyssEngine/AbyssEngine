cmake_minimum_required(VERSION 3.20)

project(abyss)

set(CMAKE_BUILD_TYPE Debug)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(abyss
    src/abyss.c
    src/globals.c src/globals.h
    src/log.c src/log.h
    src/config.c src/config.h
)

target_link_libraries(abyss
    ${SDL2_LIBRARIES}
)

target_compile_options(abyss PRIVATE -gdwarf-4)

file(GLOB files_to_copy ${CMAKE_SOURCE_DIR}/content/*)

foreach(file ${files_to_copy})
    get_filename_component(filename ${file} NAME)
    configure_file(${file} ${CMAKE_BINARY_DIR}/${filename} COPYONLY)
endforeach()
