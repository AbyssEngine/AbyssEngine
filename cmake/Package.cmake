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


function(package_abyss_engine)
endfunction()