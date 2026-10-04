cmake_minimum_required(VERSION 4.4.3)
project(tetris)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
find_package(SDL3_ttf REQUIRED CONFIG)

add_executable(tetris main.c)

target_link_libraries(tetris PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
)
