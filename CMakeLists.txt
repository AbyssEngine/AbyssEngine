cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (APPLE)
    set(LANGUAGES C OBJC)
else ()
    set(LANGUAGES C)
endif ()
project(AbyssEngine ${LANGUAGES})

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
        src/Abyss.c
        src/audio/AudioManager.c src/audio/AudioManager.h
        src/audio/AudioStream.c src/audio/AudioStream.h
        src/common/AbyssConfiguration.c src/common/AbyssConfiguration.h
        src/common/FileManager.c src/common/FileManager.h
        src/common/Globals.c src/common/Globals.h
        src/common/Logging.c src/common/Logging.h
        src/common/MpqStream.c src/common/MpqStream.h
        src/common/ResourcePaths.h
        src/common/RingBuffer.c src/common/RingBuffer.h
        src/drawing/Cursor.c src/drawing/Cursor.h
        src/drawing/Label.c src/drawing/Label.h
        src/drawing/Sprite.c src/drawing/Sprite.h
        src/scenes/Scene.c src/scenes/Scene.h
        src/scenes/SceneMainMenu.c src/scenes/SceneMainMenu.h
        src/types/DC6.c src/types/DC6.h
        src/types/Font.c src/types/Font.h
        src/types/MPQ.c src/types/MPQ.h
        src/types/MPQBlock.c src/types/MPQBlock.h
        src/types/MPQHash.c src/types/MPQHash.h
        src/types/MPQHeader.c src/types/MPQHeader.h
        src/types/Palette.c src/types/Palette.h
        src/util/BitReader.c src/util/BitReader.h
        src/util/Crypto.c src/util/Crypto.h
        src/util/Huffman.c src/util/Huffman.h
        src/util/Implode.c src/util/Implode.h
        src/util/WavDecompress.c src/util/WavDecompress.h
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
