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
project(gate_plugin LANGUAGES C CXX)

execute_process(
    COMMAND python -c "import redstonex.utils; print(redstonex.utils.get_cmake_path())"
    OUTPUT_VARIABLE PyRedStoneX_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

find_package(PyRedStoneX REQUIRED)

add_library(gate_core SHARED
    src/gate_plugin/csrc/gate_plugin.c
)

set_target_properties(gate_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/gate_plugin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/gate_plugin"
)

target_include_directories(gate_core
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gate_plugin/include
)

target_link_libraries(gate_core
    PRIVATE
        PyRedStoneX::redstonex
)

set_target_properties(gate_core PROPERTIES
    LINKER_LANGUAGE CXX
    PREFIX ""
    OUTPUT_NAME "gate_core"
)

install(TARGETS gate_core
    LIBRARY DESTINATION gate_plugin
    RUNTIME DESTINATION gate_plugin
)
