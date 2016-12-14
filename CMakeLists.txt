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
add_executable(OP_control ../src/OP_control.cpp)

# Link with YARP libraries
target_link_libraries(OP_control ${YARP_LIBRARIES})

# Install apps for yarpmanager
install(FILES app/scripts/OPcontrol_app.xml app/scripts/OPcontrol_system.xml DESTINATION ${YARP_APPLICATIONS_INSTALL_DIR})
