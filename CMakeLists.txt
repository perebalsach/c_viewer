cmake_minimum_required(VERSION 3.10)
project(viewer)

# Set C standard to C11
set(CMAKE_C_STANDARD 11)

# Specify the path to the SDL2 library (change the path accordingly)
set(SDL2_LIBRARY_PATH "/opt/homebrew/Cellar/sdl2/2.28.3/lib")

# Include SDL2 headers
set(SDL2_INCLUDE_DIR "/opt/homebrew/Cellar/sdl2/2.28.3/include")
include_directories(${SDL2_INCLUDE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# Create a list of your source files in the src folder
file(GLOB SOURCES "src/*.c")

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Create the executable
add_executable(viewer ${SOURCES})

# Link SDL2 libraries
target_link_libraries(viewer ${SDL2_LIBRARIES})

# Add linker flags
target_link_options(viewer PRIVATE -lm -lSDL2)

target_link_directories(viewer PRIVATE ${SDL2_LIBRARY_PATH})
