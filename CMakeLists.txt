cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)

project(JunctionBuilder VERSION 1.1.2 LANGUAGES CXX)

# Automatic project versioning
set(PROJECT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(PROJECT_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(PROJECT_VERSION_PATCH ${PROJECT_VERSION_PATCH})

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/Version.h.in
    ${CMAKE_BINARY_DIR}/generated/Version.h
    @ONLY
)

# Path to ObjectARX SDK
set(ARX_SDK "C:/Autodesk/ObjectArxSDK2024")

# Collect source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

# Define target
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "JunctionBuilder"
    SUFFIX ".arx"
    LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/JunctionBuilder.def"
)

# RESOURCE FILES
set(RESOURCE_FILE ${CMAKE_SOURCE_DIR}/resource/dialog.rc)
set_source_files_properties(${RESOURCE_FILE} PROPERTIES LANGUAGE RC)

target_sources(${PROJECT_NAME} PRIVATE ${RESOURCE_FILE})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/resource
    ${CMAKE_BINARY_DIR}/generated
    ${ARX_SDK}/inc
    ${ARX_SDK}/inc-x64
)

# Linker directories (x64 version)
target_link_directories(${PROJECT_NAME} PRIVATE ${ARX_SDK}/lib-x64)

# Add OpenXLSX
set(OPENXLSX_LIBRARY_TYPE "STATIC" CACHE STRING "Type of library to build for OpenXLSX")
set(OPENXLSX_BUILD_TESTS OFF CACHE BOOL "Whether to build tests for OpenXLSX")
set(OPENXLSX_BUILD_SAMPLES OFF CACHE BOOL "Whether to build samples for OpenXLSX")
set(OPENXLSX_BUILD_BENCHMARKS OFF CACHE BOOL "Whether to build benchmarks for OpenXLSX")
add_subdirectory(external/OpenXLSX)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    accore.lib
    ac1st24.lib
    rxapi.lib
    acgiapi.lib
    acdb24.lib
    acad.lib
    acge24.lib
    AcPal.lib
    acgeoment.lib
    OpenXLSX::OpenXLSX
)

# Required preprocessor macros for ARX
target_compile_definitions(${PROJECT_NAME} PRIVATE
    _USRDLL
    _ARX_
)

# Match Visual Studio default runtime (important for ARX compatibility)
set_target_properties(${PROJECT_NAME} PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)

target_compile_options(${PROJECT_NAME} PRIVATE /MD)
target_compile_options(${PROJECT_NAME} PRIVATE /Zc:wchar_t)
target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)


# Optional: suppress common 64-bit pointer warnings that ObjectARX triggers
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4311 /wd4312)
endif()
