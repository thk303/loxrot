cmake_minimum_required(VERSION 3.10)
project(loxrot VERSION "0.7")

# if(CMAKE_BUILD_TYPE STREQUAL "Release")
#     message("Configuring Release build")

#     # Beispiel: Optimierung für Geschwindigkeit (-O3 bei GCC/Clang)
#     set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

#     # Beispiel: Zusätzliche Optionen für GCC/Clang
#     set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

#     # Optional: Linker-Optimierungen aktivieren
#     set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--strip-all")
# endif()

set(CMAKE_CXX_STANDARD 20)
add_definitions(-DUNICODE)
include_directories(loxrot)
file(GLOB SOURCES "loxrot/*.cpp")
add_executable(loxrot ${SOURCES})

target_compile_options(loxrot PRIVATE
    $<$CONFIG:Release>:/MT
)
