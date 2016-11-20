cmake_minimum_required(VERSION 3.6)
project(labirint_git)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(SOURCE_FILES
    game.c
    game.h
    labirintw.c
    Makefile)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(labirint_git ${SOURCE_FILES})

target_link_libraries(labirint_git ${CURSES_LIBRARIES})