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
find_package(pybind11 REQUIRED)

if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# RedStoneX
set(REDSTONEX_INSTALL OFF)
add_subdirectory(extern/RedStoneX)

# PyRedStoneX
pybind11_add_module(_core 
    src/redstonex/csrc/core.cpp
)

target_include_directories(_core 
    PRIVATE 
        src
        src/redstonex/include
)

target_link_libraries(_core 
    PRIVATE 
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
    TARGETS
        redstonex
        _core

    LIBRARY DESTINATION redstonex
    RUNTIME DESTINATION redstonex
)
