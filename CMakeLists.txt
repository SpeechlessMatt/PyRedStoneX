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

cmake_minimum_required(VERSION 3.12)
project(PyRedStoneX)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(REDSTONEX_SRCS
    extern/RedStoneX/src/redstonex_sim.c
    extern/RedStoneX/src/redstonex_obj.c
    extern/RedStoneX/src/redstonex_components.c
)

add_library(redstonex STATIC ${REDSTONEX_SRCS})
target_include_directories(redstonex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/RedStoneX/include)
set_target_properties(redstonex PROPERTIES POSITION_INDEPENDENT_CODE ON)

pybind11_add_module(_core src/core.cpp)
pybind11_strip(_core)

target_include_directories(_core PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/RedStoneX/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(_core PRIVATE redstonex)

set_target_properties(_core PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/redstonex"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/redstonex"
)
