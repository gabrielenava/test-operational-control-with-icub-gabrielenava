## CMakeLists for Make-it-Roll code
## Author Gabriele Nava
## Date Dec 2016

# CMake minimum version required
cmake_minimum_required(VERSION 2.8.9)

# Name of this project
project(make-it-roll)

# Find YARP 
find_package(YARP REQUIRED)

# Add YARP include directories
include_directories(${YARP_INCLUDE_DIRS})

# Set up our program
add_executable(OP_control ${CMAKE_SOURCE_DIR}/src/OP_control.cpp)

# Link with YARP and control libraries
target_link_libraries(OP_control ${YARP_LIBRARIES})

# OPTIONAL: install apps for yarpmanager
option(INSTALL_YARPMANAGER_APPS "Install apps for yarpmanager" FALSE)

if (${INSTALL_YARPMANAGER_APPS})
    install(FILES app/scripts/OPcontrol_app.xml app/scripts/OPcontrol_system.xml DESTINATION ${YARP_APPLICATIONS_INSTALL_DIR})
endif()

install(CODE "MESSAGE(\"This repository simply acts as a container\")")

# Build example test (not working because of a change in robot-testing repo)
#add_subdirectory(tests)
#add_subdirectory(app)

