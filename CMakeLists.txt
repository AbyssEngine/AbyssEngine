cmake_minimum_required(VERSION 3.20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(AbyssEngine)

include(Package)
include(BuildOptions)
include(Dependencies)
include(SetupSourceGroups)

add_executable(${PROJECT_NAME} WIN32)
target_sources(${PROJECT_NAME} PRIVATE src/AbyssEngine.c)
add_subdirectory(src)
setup_source_groups(src)

if (APPLE)
    find_library(OSX_VIDEOTOOLBOX VideoToolbox)
    find_library(OSX_COREMEDIA CoreMedia)
    find_library(OSX_SECURITY Security)
    set(ADDITIONAL_LIBRARIES
            ${OSX_VIDEOTOOLBOX}
            ${OSX_COREMEDIA}
            ${OSX_SECURITY}
    )
elseif (UNIX)
    set(ADDITIONAL_LIBRARIES "m")
endif ()

target_link_libraries(AbyssEngine PRIVATE ${ABYSS_DEPENDENCIES} ${ADDITIONAL_LIBRARIES})

include(AutoClangFormat)

if (APPLE)
    if (NOT DEFINED GITHUB_ACTIONS)
        # Add the clang-format target as a dependency for Xcode
        add_dependencies(${PROJECT_NAME} clang-format)
    endif ()

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


package_abyss_engine()
add_subdirectory(tests)
