cmake_minimum_required(VERSION 3.15)

project(rng_hunter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Source files
set(SOURCES
    main.cpp
    seed_parser.cpp
    rng_table.cpp
    rng_hunter.cpp
    msvc_rand_wrapper.cpp
    rng_sim.cpp
    walkthrough_gen/walkthrough_gen.cpp)

# Header files
set(HEADERS
    CLI11.hpp
    msvc_rand_wrapper.h
    rng_hunter.h
    rng_sim.h
    rng_table.h
    seed_parser.h
    nlohmann/json.hpp
    inja/inja.hpp
    walkthrough_gen/walkthrough_gen.h )


file(GLOB TEMPLATE_FILES 
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/walkthrough_gen/templates/*.md"
)

# Generate header with dependency tracking
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/generated/templates.h"
    COMMAND ${CMAKE_COMMAND}
        -DTEMPLATE_DIR="${CMAKE_SOURCE_DIR}/walkthrough_gen/templates"
        -DOUTPUT_FILE="${CMAKE_BINARY_DIR}/generated/templates.h"
        -P "${CMAKE_SOURCE_DIR}/cmake/generate_templates.cmake"
    DEPENDS ${TEMPLATE_FILES}
    COMMENT "Generating embedded templates"
) 

add_custom_target(generate_templates ALL
    DEPENDS "${CMAKE_BINARY_DIR}/generated/templates.h"
)
 
# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

add_dependencies(rng_hunter generate_templates)
target_include_directories(rng_hunter 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/generated
)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

# Compiler-specific flags
if(MSVC)
    # MSVC-specific flags
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W3                     # Warning level 3
        /permissive-           # Standards conformance
        $<$<CONFIG:Release>:/O2>      # Optimize for speed
        $<$<CONFIG:Release>:/GL>      # Whole program optimization
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/LTCG>    # Link-time code generation
    )
    # Set subsystem to Console
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
else()
    # GCC/Clang flags
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -pedantic
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-march=native>
    )
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CONSOLE
        UNICODE
        _UNICODE
    )
endif()


# Installation rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Print configuration summary
message(STATUS "")
message(STATUS "CTRNGHunter Configuration:")
message(STATUS "  CMake version: ${CMAKE_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")