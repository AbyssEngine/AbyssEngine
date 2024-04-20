if (NOT DEFINED GITHUB_ACTIONS)
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
endif ()