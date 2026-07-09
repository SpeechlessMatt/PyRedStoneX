# Copyright (C) 2026 Czy_4201b
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.15)

project(
    PyRedStoneX
    LANGUAGES C CXX
)

# Python/pybind11
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)

if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# RedStoneX
add_subdirectory(extern/RedStoneX EXCLUDE_FROM_ALL)

# PyRedStoneX
pybind11_add_module(_core 
    src/redstonex/csrc/core.cpp
)

target_include_directories(_core 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/redstonex/include
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/RedStoneX/include
)

target_link_libraries(_core 
    PUBLIC
        redstonex
)

# Linux/macOS runtime search path
if(UNIX)
    if(APPLE)
        set_target_properties(_core PROPERTIES
            INSTALL_RPATH "@loader_path"
            BUILD_RPATH "@loader_path"
        )
    else()
        set_target_properties(_core PROPERTIES
            INSTALL_RPATH "$ORIGIN"
            BUILD_RPATH "$ORIGIN"
        )
    endif()
endif()

# install
include(GNUInstallDirs)

install(
    TARGETS _core
    LIBRARY DESTINATION redstonex
)

install(
    TARGETS redstonex
    EXPORT PyRedStoneXTargets
    LIBRARY DESTINATION redstonex
    ARCHIVE DESTINATION redstonex
    RUNTIME DESTINATION redstonex
)

install(
    DIRECTORY extern/RedStoneX/include/
    DESTINATION redstonex/include
)

install(
    DIRECTORY src/redstonex/include/
    DESTINATION redstonex/include
)

set_target_properties(redstonex PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/redstonex/include>"
)

install(
    EXPORT PyRedStoneXTargets
    FILE PyRedStoneXTargets.cmake
    NAMESPACE PyRedStoneX::
    DESTINATION redstonex/cmake
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/PyRedStoneXConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PyRedStoneXConfig.cmake"
    INSTALL_DESTINATION redstonex/cmake
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/PyRedStoneXConfig.cmake"
    DESTINATION redstonex/cmake
)
