cmake_minimum_required(VERSION 3.20)

project(AbyssEngine C)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)

#add_compile_options(-fsanitize=address -g)
#add_link_options(-fsanitize=address)

# Ensures that we do an out of source build
MACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD MSG)
    STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
            "${CMAKE_BINARY_DIR}" insource)
    GET_FILENAME_COMPONENT(PARENTDIR ${CMAKE_SOURCE_DIR} PATH)
    STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
            "${PARENTDIR}" insourcesubdir)
    IF (insource OR insourcesubdir)
        MESSAGE(FATAL_ERROR "${MSG}")
    ENDIF (insource OR insourcesubdir)
ENDMACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)
MACRO_ENSURE_OUT_OF_SOURCE_BUILD(
        "${CMAKE_PROJECT_NAME} requires an out of source build."
)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, defaulting to Release")
    set(CMAKE_BUILD_TYPE Release)
endif ()

add_subdirectory(3rdparty EXCLUDE_FROM_ALL)

add_executable(AbyssEngine WIN32 MACOSX_BUNDLE
        src/abyss.c
        src/common/resource_paths.h
        src/common/globals.c src/common/globals.h
        src/common/log.c src/common/log.h
        src/common/config.c src/common/config.h
        src/types/mpq.c src/types/mpq.h
        src/types/mpq_hash.c src/types/mpq_hash.h
        src/types/mpq_header.c src/types/mpq_header.h
        src/types/mpq_block.c src/types/mpq_block.h
        src/common/mpq_stream.c src/common/mpq_stream.h
        src/util/crypto.c src/util/crypto.h
        src/common/fileman.c src/common/fileman.h
        src/util/implode.c src/util/implode.h
        src/types/palette.c src/types/palette.h
        src/types/dc6.c src/types/dc6.h
        src/drawing/sprite.c src/drawing/sprite.h
        src/drawing/cursor.c src/drawing/cursor.h
        src/types/font.c src/types/font.h
        src/scenes/scene.c src/scenes/scene.h
        src/scenes/scene_mainmenu.c src/scenes/scene_mainmenu.h
        src/types/font.c src/types/font.h
)
# if WIN32 then SDL2::SDL2main needs to be linked
if (WIN32)
    set(ADDITIONAL_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
endif ()

target_link_libraries(AbyssEngine
        SDL2::SDL2
        zlibstatic
        archive_static
        ${ADDITIONAL_LIBRARIES}
)

if (APPLE)
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
