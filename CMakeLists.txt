cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(AbyssEngine)

enable_testing()

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function(setup_source_groups curdir)
    file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${curdir} ${curdir}/*)
    foreach (child ${children})
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/${child})
            setup_source_groups(${curdir}/${child})
        else ()
            string(REPLACE "/" "\\" group_name ${curdir})
            source_group(${group_name} FILES ${curdir}/${child})
        endif ()
    endforeach ()
endfunction()

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

add_executable(${PROJECT_NAME} WIN32)
target_sources(${PROJECT_NAME} PRIVATE src/Abyss.c)
add_subdirectory(src)
setup_source_groups(src)


# if WIN32 then SDL2::SDL2main needs to be linked
if (WIN32)
    set(ADDITIONAL_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
endif ()

if (APPLE)
    find_library(OSX_VIDEOTOOLBOX VideoToolbox)
    find_library(OSX_COREMEDIA CoreMedia)
    find_library(OSX_SECURITY Security)
    set(ADDITIONAL_LIBRARIES
            ${OSX_VIDEOTOOLBOX}
            ${OSX_COREMEDIA}
            ${OSX_SECURITY}
    )
endif ()

target_link_libraries(AbyssEngine
        ${SDL2_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${LibArchive_LIBRARIES}
        ${FFMPEG_LIBRARIES}
        ${ADDITIONAL_LIBRARIES}
)

if (UNIX)
    # Find clang-format executable
    find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)

    # Check if clang-format executable is found
    if (NOT CLANG_FORMAT_EXECUTABLE)
        message(FATAL_ERROR "clang-format not found. Please install clang-format or specify its path manually.")
    endif ()

    # Find all C and H files in the src directory
    file(GLOB_RECURSE ALL_SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.c" "${CMAKE_SOURCE_DIR}/src/*.h")

    # Define a custom target and command to run clang-format
    add_custom_target(
            clang-format
            COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${ALL_SOURCE_FILES}
            COMMENT "Running clang-format on all C and H files"
            VERBATIM
    )
endif ()

if (APPLE)
    # Add the clang-format target as a dependency for Xcode
    add_dependencies(${PROJECT_NAME} clang-format)


    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/extra/macos/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/AbyssEngine.app/Contents/Resources")
    set_target_properties(AbyssEngine PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME "Abyss Engine"
            MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
            PRODUCT_BUNDLE_IDENTIFIER "com.abyssengine"
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


add_subdirectory(tests)

#include(InstallRequiredSystemLibraries)
#
#install(TARGETS AbyssEngine
#        RUNTIME DESTINATION dist
#        BUNDLE DESTINATION ../MacOS
#)
#
#set(CPACK_PACKAGE_NAME "AbyssEngine")
#set(CPACK_BUNDLE_NAME "AbyssEngine")
#set(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/extra/macos/MacOSXBundleInfo.plist.in")
#set(CPACK_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/extra/macos/icon.icns")
#set(CPACK_PACKAGE_VENDOR "AbyssEngine Developers")
#set(CPACK_PACKAGE_VERSION_MAJOR 0)
#set(CPACK_PACKAGE_VERSION_MINOR 1)
#set(CPACK_PACKAGE_VERSION_PATCH 0)
#set(CPACK_NSIS_MUI_ICON "@CMake_SOURCE_DIR@/extra/icon.ico")
#include(CPack)