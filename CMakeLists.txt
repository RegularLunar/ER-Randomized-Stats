cmake_minimum_required(VERSION 3.24)
project(ER-RandomizedStats LANGUAGES CXX)


add_library(ER-RandomizedStats SHARED
    src/dllmain.cpp
)

set_target_properties(ER-RandomizedStats PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_compile_features(ER-RandomizedStats
    PUBLIC
        cxx_std_17
)

target_include_directories(ER-RandomizedStats
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(ER-RandomizedStats
    PUBLIC
        ERRANDOMIZEDSTATS_EXPORTS
        _WINDOWS
        _USRDLL
        _MBCS
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        $<$<STREQUAL:${CMAKE_CXX_COMPILER_ARCHITECTURE_ID},X86>:WIN32>
)

target_link_libraries(ER-RandomizedStats
    PUBLIC
        Psapi.lib
)

target_compile_options(ER-RandomizedStats
    PUBLIC
        /W3
)

