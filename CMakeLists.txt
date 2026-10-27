cmake_minimum_required(VERSION 3.15)
project(tierjit CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(APPLE)
    execute_process(
        COMMAND brew --prefix llvm
        OUTPUT_VARIABLE LLVM_HOMEBREW_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(LLVM_HOMEBREW_PATH)
        list(APPEND CMAKE_PREFIX_PATH "${LLVM_HOMEBREW_PATH}/lib/cmake/llvm")
    endif()
endif()
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(tierjit src/main.cpp)
llvm_map_components_to_libnames(llvm_libs support core orcjit native)
target_link_libraries(tierjit PRIVATE ${llvm_libs})
if(APPLE AND LLVM_HOMEBREW_PATH)
    set_target_properties(tierjit PROPERTIES
        INSTALL_RPATH "${LLVM_HOMEBREW_PATH}/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()
