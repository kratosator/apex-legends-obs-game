cmake_minimum_required(VERSION 3.5)

project(apex-game VERSION 1.5.0)

include(CreateResources.cmake)

find_package(libobs REQUIRED)
find_package(Leptonica REQUIRED)

create_resources(images src/images.c src/images.h)

set(apex-game_SOURCES src/apex-game.c src/images.c)

add_library(apex-game MODULE ${apex-game_SOURCES})

target_link_libraries(apex-game ${LIBOBS_LIBRARIES} ${Leptonica_LIBRARIES})

include_directories(${LIBOBS_INCLUDE_DIR})
include_directories("${Leptonica_INCLUDE_DIRS}")
