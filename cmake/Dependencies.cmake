find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(ZLIB REQUIRED)

find_package(LibArchive REQUIRED)
include_directories(${LibArchive_INCLUDE_DIR})

find_package(FFMPEG COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE SWRESAMPLE REQUIRED)
include_directories(${FFMPEG_INCLUDE_DIRS})

set(ABYSS_DEPENDENCIES
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        ZLIB::ZLIB
        ${LibArchive_LIBRARIES}
        ${FFMPEG_LIBRARIES}
        ${ADDITIONAL_LIBRARIES}
)