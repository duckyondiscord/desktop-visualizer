cmake_minimum_required(VERSION 3.7)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

project("Desktop Visualizer")
set(EXECUTABLE_NAME "visualino")

add_executable(${EXECUTABLE_NAME} main.cpp)

# X11
find_package(X11)
include_directories(${X11_INCLUDE_DIR})
target_link_libraries(${EXECUTABLE_NAME} ${X11_LIBRARIES})

# fftw
find_package(FFTW3)
include_directories(${FFTW3_INCLUDE_DIRS})
target_link_libraries(${EXECUTABLE_NAME} ${FFTW3_LIBRARIES})
target_link_directories(${EXECUTABLE_NAME} PUBLIC ${FFTW3_LIBRARY_DIRS})

# PulseAudio
find_package(PulseAudio)
include_directories(${PULSEAUDIO_INCLUDE_DIR})
target_link_libraries(${EXECUTABLE_NAME} ${PULSEAUDIO_LIBRARY})
target_link_libraries(${EXECUTABLE_NAME} pulse-simple)

# pthread
find_package(Threads)
target_link_libraries (${EXECUTABLE_NAME} ${CMAKE_THREAD_LIBS_INIT})

# inih
target_link_libraries (${EXECUTABLE_NAME} INIReader)

# SDL2
find_package (SDL2 REQUIRED)
find_package (SDL2_image REQUIRED)

target_link_libraries (${EXECUTABLE_NAME} ${SDL2_LIBRARIES})
target_link_libraries (${EXECUTABLE_NAME} SDL2_image)

install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)
