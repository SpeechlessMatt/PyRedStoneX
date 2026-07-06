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

pybind11_add_module(_core core.cpp)
pybind11_strip(_core)

target_include_directories(_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/RedStoneX/include)
target_link_libraries(_core PRIVATE redstonex)

set_target_properties(_core PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/redstonex"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/redstonex"
)
