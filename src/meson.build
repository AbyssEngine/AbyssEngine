build_opts = [
    '-D_ISOC11_SOURCE', '-D_XOPEN_SOURCE=700', '-U__STRICT_ANSI__',

    # Warnings
    '-Wundef', '-Wmissing-prototypes', '-Wshadow', '-Wparentheses',
    '-Wpointer-arith', '-Wno-pointer-sign',

    # Warnings to treat as errors
    '-Werror=implicit-function-declaration',
]

# Required dependencies
dependencies = [
    dependency('zlib'),
    dependency('sdl2'),
]

sources = [
    'abyss.c',

    'common/config.c',
    'common/fileman.c',
    'common/globals.c',
    'common/log.c',
    'common/mpq_stream.c',

    'drawing/cursor.c',
    'drawing/sprite.c',

    'scenes/scene.c',
    'scenes/scene_mainmenu.c',

    'types/dc6.c',
    'types/font.c',
    'types/mpq.c',
    'types/mpq_block.c',
    'types/mpq_hash.c',
    'types/mpq_header.c',
    'types/palette.c',

    'util/crypto.c',
    'util/implode.c',
]

add_global_arguments(build_opts, language: 'c')

executable('AbyssEngine',
    install: true,
    sources: sources,
    dependencies: dependencies,
)
