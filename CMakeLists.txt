cmake_minimum_required(VERSION 4.4.3)

project(tetris LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

add_executable(tetris
    src/main.c
    src/tetris.c
    src/render.c
    src/input.c
)

target_link_libraries(tetris PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
)

if (EMSCRIPTEN)
    set_target_properties(tetris PROPERTIES SUFFIX ".html")

    target_link_options(tetris PRIVATE
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
    )
else()
    add_custom_command(TARGET tetris POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:tetris>/assets"
    )
endif()
