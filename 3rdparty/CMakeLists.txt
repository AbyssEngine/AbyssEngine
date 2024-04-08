set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(libarchive EXCLUDE_FROM_ALL "")
add_subdirectory(zlib EXCLUDE_FROM_ALL)
add_subdirectory(sdl EXCLUDE_FROM_ALL)