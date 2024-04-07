cmake_minimum_required(VERSION 3.20)

project(abyss)

set(CMAKE_BUILD_TYPE Debug)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(ZLIB REQUIRED)
find_package(LibArchive REQUIRED)

add_executable(abyss
    src/abyss.c
    src/globals.c src/globals.h
    src/log.c src/log.h
    src/config.c src/config.h
    src/mpq.c src/mpq.h
    src/mpq_hash.c src/mpq_hash.h
    src/mpq_header.c src/mpq_header.h
    src/mpq_block.c src/mpq_block.h
    src/mpq_stream.c src/mpq_stream.h
    src/crypto.c src/crypto.h
    src/fileman.c src/fileman.h
    src/implode.c src/implode.h
    src/palette.c src/palette.h
    src/dc6.c src/dc6.h
)

target_link_libraries(abyss
    ${SDL2_LIBRARIES}
    ZLIB::ZLIB
    LibArchive::LibArchive
)

target_compile_options(abyss PRIVATE -gdwarf-4)

file(GLOB files_to_copy ${CMAKE_SOURCE_DIR}/content/*)

foreach(file ${files_to_copy})
    get_filename_component(filename ${file} NAME)
    configure_file(${file} ${CMAKE_BINARY_DIR}/${filename} COPYONLY)
endforeach()
