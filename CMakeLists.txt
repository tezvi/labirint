cmake_minimum_required(VERSION 3.6)
project(labirint)

set(CMAKE_C_FLAGS "-Wall -O2 -ggdb3")

set(SOURCE_FILES
        game.c
        game.h
        labirintw.c)

find_package(Curses REQUIRED)
find_library(MENU_LIBRARY menu)

include_directories(${CURSES_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${MENU_LIBRARY} ${CURSES_LIBRARIES})