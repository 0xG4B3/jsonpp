cmake_minimum_required(VERSION 3.20)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("jsonpp" 
    VERSION 1.0.0
    DESCRIPTION "Modern C++23 header-only JSON library"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(jsonpp_lib INTERFACE)
target_include_directories(jsonpp_lib INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(jsonpp_lib INTERFACE cxx_std_23)

add_executable(jsonpp "jsonpp.cpp")
target_link_libraries(jsonpp PRIVATE jsonpp_lib)

if(MSVC)
    target_compile_options(jsonpp PRIVATE /W4)

else()
    target_compile_options(jsonpp PRIVATE -Wall -Wextra -Wpedantic)
endif()
