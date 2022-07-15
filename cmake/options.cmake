
# Build Options
option(VOICE_VISUALIZER_BUILD_EXAMPLES "Build Voice Visualizer Example" ON)
option(GLFW_USE_WAYLAND "Build GLFW for Wayland" OFF)

# Versions
if(NOT GLFW_TAG)
    set(GLFW_TAG 3.3.2)
endif()

# Path variables
set(THIRD_PARTY_DIR          ${CMAKE_SOURCE_DIR}/third_party)
set(EXT_CMAKE_STAGING_PREFIX ${CMAKE_BINARY_DIR}/staging${CMAKE_INSTALL_PREFIX})
