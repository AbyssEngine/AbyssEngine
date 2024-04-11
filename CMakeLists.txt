cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (APPLE)
    set(LANGUAGES C OBJC)
else ()
    set(LANGUAGES C)
endif ()
project(AbyssEngine LANGUAGES ${LANGUAGES})

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, defaulting to Release")
    set(CMAKE_BUILD_TYPE Release)
endif ()

#turn on all warnings
if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

if (CMAKE_C_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
endif ()

if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-gnu-zero-variadic-macro-arguments")
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Homebrew ships libarchive keg only, include dirs have to be set manually
    execute_process(
            COMMAND brew --prefix libarchive
            OUTPUT_VARIABLE LIBARCHIVE_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            COMMAND_ERROR_IS_FATAL ANY
    )
    set(LibArchive_INCLUDE_DIR "${LIBARCHIVE_PREFIX}/include")
endif ()

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

find_package(LibArchive REQUIRED)
include_directories(${LibArchive_INCLUDE_DIR})

find_package(FFMPEG COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE SWRESAMPLE REQUIRED)
include_directories(${FFMPEG_INCLUDE_DIRS})

#add_compile_options(-fsanitize=address -g)
#add_link_options(-fsanitize=address)

add_executable(AbyssEngine WIN32 MACOSX_BUNDLE
        src/abyss.c
        src/audio/audioman.c src/audio/audioman.h
        src/audio/audio_stream.c src/audio/audio_stream.h
        src/common/config.c src/common/config.h
        src/common/fileman.c src/common/fileman.h
        src/common/globals.c src/common/globals.h
        src/common/log.c src/common/log.h
        src/common/mpq_stream.c src/common/mpq_stream.h
        src/common/resource_paths.h
        src/common/ring_buffer.c src/common/ring_buffer.h
        src/drawing/cursor.c src/drawing/cursor.h
        src/drawing/label.c src/drawing/label.h
        src/drawing/sprite.c src/drawing/sprite.h
        src/scenes/scene.c src/scenes/scene.h
        src/scenes/scene_mainmenu.c src/scenes/scene_mainmenu.h
        src/types/dc6.c src/types/dc6.h
        src/types/font.c src/types/font.h
        src/types/mpq.c src/types/mpq.h
        src/types/mpq_block.c src/types/mpq_block.h
        src/types/mpq_hash.c src/types/mpq_hash.h
        src/types/mpq_header.c src/types/mpq_header.h
        src/types/palette.c src/types/palette.h
        src/util/crypto.c src/util/crypto.h
        src/util/implode.c src/util/implode.h
        src/util/huffman.c src/util/huffman.h
)
# if WIN32 then SDL2::SDL2main needs to be linked
if (WIN32)
    set(ADDITIONAL_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
endif ()

target_link_libraries(AbyssEngine
        ${SDL2_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${LibArchive_LIBRARIES}
        ${FFMPEG_LIBRARIES}
        ${ADDITIONAL_LIBRARIES}
)

if (APPLE)
    find_library(OSX_VIDEOTOOLBOX VideoToolbox)
    find_library(OSX_COREMEDIA CoreMedia)
    find_library(OSX_SECURITY Security)

    set_target_properties(AbyssEngine PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "Abyss Engine" ,
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.abyss" ,
            MACOSX_BUNDLE_ICON_FILE "icon.icns"
    )

    find_library(OSX_VIDEOTOOLBOX VideoToolbox)
    find_library(OSX_COREMEDIA CoreMedia)
    find_library(OSX_SECURITY Security)
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/extra/macos/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/AbyssEngine.app/Contents/Resources")
    set_target_properties(AbyssEngine PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME "Abyss Engine"
            MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.abyssengine"
            MACOSX_BUNDLE_ICON_FILE "icon.icns"
            #MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/extra/macos/MacOSXBundleInfo.plist.in"
            MACOSX_BUNDLE_COPYRIGHT "(c) 2023 Timothy Sarbin"
    )

    set_source_files_properties("extra/macos/icon.icns" PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )

    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install path prefix, prepended onto install directories" FORCE)
else ()
    add_custom_command(TARGET AbyssEngine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/content $<TARGET_FILE_DIR:AbyssEngine>)
endif ()
