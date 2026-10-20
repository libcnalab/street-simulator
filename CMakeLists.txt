# =============================================================================
# cna-street -- a realistic 3D city street rendered with the CNA framework.
# =============================================================================
# Application sources live under street/ so CNA's module-layout validation
# sees its own source tree when it is added as a subdirectory.
cmake_minimum_required(VERSION 3.23)

project(CnaStreet
    VERSION 0.1.0
    DESCRIPTION "A realistic 3D city street built on the CNA framework"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# The renderer CNA compiles in. OPENGL33 is the default here rather than CNA's
# own Linux default (OPENGLES3) because the street's material set uses desktop
# GL 3.3 shading; every renderer in CNA's 2d3d set is selectable.
# A browser has no desktop GL: an Emscripten build targets WebGL 2, which runs
# the same GLSL ES 3.00 the street's own shaders are written to.
if(EMSCRIPTEN)
    set(_cna_street_default_renderer "WEBGL2")
    set(_cna_street_default_tests OFF)
else()
    set(_cna_street_default_renderer "OPENGL33")
    set(_cna_street_default_tests ON)
endif()
set(CNA_STREET_RENDERER "${_cna_street_default_renderer}" CACHE STRING
    "CNA graphics renderer to build against (OPENGL33, OPENGLES3, VULKAN, WEBGL2, SOFTWARE, ...)")

option(CNA_STREET_BUILD_TESTS "Build cna-street's unit tests" ${_cna_street_default_tests})

include(CnaStreetWarnings)
include(CnaStreetDependencies)

message(STATUS "cna-street: ${PROJECT_VERSION}, renderer ${CNA_STREET_RENDERER}, "
               "${CMAKE_BUILD_TYPE}, ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "cna-street: CNA at ${CNA_ROOT_DIR}")

# cna-engine links CNA's static module targets. With CNA's default shared
# library the `CNA` umbrella would be a second copy of CNA in the same process
# (cna-engine upstream-issues.md UP-006), so CNA is built statically here.
set(CNA_SHARED_LIBRARY OFF CACHE BOOL "" FORCE)

add_subdirectory("${CNA_ROOT_DIR}" cna EXCLUDE_FROM_ALL)

# cna-engine (a sibling of CNA by default): its strict modules and the enhanced
# graphics module over CNA's PbrEffect. cna-extended is not used.
set(CNA_STREET_CNA_ENGINE_ROOT "${CNA_ROOT_DIR}/../cna-engine" CACHE PATH
    "Path to the cna-engine checkout")
cna_street_require_checkout("${CNA_STREET_CNA_ENGINE_ROOT}" "cna-engine"
    "https://github.com/libcna/cna-engine.git" "develop")
set(CNA_ENGINE_BUILD_TESTS            OFF CACHE BOOL "" FORCE)
set(CNA_ENGINE_BUILD_EXAMPLES         OFF CACHE BOOL "" FORCE)
set(CNA_ENGINE_WITH_EXTENDED          OFF CACHE BOOL "" FORCE)
set(CNA_ENGINE_WITH_ENHANCED_GRAPHICS ON  CACHE BOOL "" FORCE)
set(CNA_ENGINE_INSTALL                OFF CACHE BOOL "" FORCE)
add_subdirectory("${CNA_STREET_CNA_ENGINE_ROOT}" cna-engine EXCLUDE_FROM_ALL)
message(STATUS "cna-street: cna-engine at ${CNA_STREET_CNA_ENGINE_ROOT}")

# CNA's SDL comes from a vendored submodule built as part of the framework; a
# few ancillary upstream targets still expect the config packages to be found.
foreach(_sdl_package IN ITEMS SDL3)
    if(NOT TARGET ${_sdl_package}::${_sdl_package})
        find_package(${_sdl_package} QUIET CONFIG)
    endif()
endforeach()

add_subdirectory(street)

# The content pipeline runs host tools over the source tree; a browser build
# has neither, and generates its surfaces at start-up instead.
if(NOT EMSCRIPTEN)
    include(CnaStreetContent)
endif()

if(CNA_STREET_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
