cmake_minimum_required(VERSION 3.10)
project(litestat VERSION 0.1 LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Library target
file(GLOB LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_library(litestat SHARED ${LIB_SOURCES})

target_include_directories(litestat
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Set library properties
set_target_properties(litestat PROPERTIES
    OUTPUT_NAME "litestat"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Export and install (optional)
include(GNUInstallDirs)
install(TARGETS litestat
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
