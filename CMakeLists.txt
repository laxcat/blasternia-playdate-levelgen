# SETUP
cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# PROJECT
project(blasternia_level_gen VERSION 0.1.0 LANGUAGES CXX C OBJCXX OBJC)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT DEFINED DEV_INTERFACE)
    set(DEV_INTERFACE 1)
endif()

set(CMAKE_SKIP_INSTALL_RULES ON QUIET)
set(BX_SILENCE_DEBUG_OUTPUT OFF)

# MAIN ENGINE
set(SetupLib_include_dirs)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/engine")

# EXE
include_directories(${SetupLib_include_dirs})
set(EXE_NAME blastlg)
add_executable(${EXE_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Playdate_proj/src/common/rng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Playdate_proj/src/game/leveldata.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Playdate_proj/src/game/priority.c
)
target_compile_definitions(${EXE_NAME} PUBLIC DEV_INTERFACE=${DEV_INTERFACE})
target_link_libraries(${EXE_NAME} "game_project_engine" ${SetupLib_libs})

# COPY ASSETS
file(GLOB ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/[!.]*")
file(COPY ${ASSET_FILES} DESTINATION "${CMAKE_BINARY_DIR}/assets")
