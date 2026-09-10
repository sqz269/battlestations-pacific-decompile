# Deferred source registry for modules added on worker branches.
#
# CMakeLists.txt lists sources directly and is owned by the integrator while a packet is
# open, so worker branches register their sources here instead and never touch it. This file
# is pulled in through CMAKE_PROJECT_INCLUDE (set by scripts/build.ps1), which runs right after
# project(); the calls below are deferred to the end of the top-level directory so the targets
# already exist. Append one registration per module; keep the list sorted by module name.
#
# Registration forms:
#   cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/<module>.cpp)
#   cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL add_executable bsp_<probe> src/<probe>.cpp)
#   cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_link_libraries bsp_<probe> PRIVATE bsp_core)
#
# The integrator folds entries into CMakeLists.txt when the owning packet closes.
cmake_minimum_required(VERSION 3.19)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/app_bootstrap.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/app_frame.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/app_shutdown.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/audio_online_startup.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/game_entry.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/input_settings.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/locale_tables.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/native_string.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/platform_window.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/storage_pool.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/vfs_startup.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/winmain_startup.cpp)
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_core PRIVATE src/gui_startup.cpp)
