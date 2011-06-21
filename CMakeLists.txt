# ===============================================================
# = CMake file for image processor - Jess Robertson, 2010-06-14 =
# ===============================================================
project(process_images)

# Configure CMake build
cmake_minimum_required(VERSION 2.8) 
include(FindPackageHandleStandardArgs) 
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "modules") 
set(CMAKE_VERBOSE_MAKEFILE false)
set(CMAKE_C_COMPILER gcc) 
set(CMAKE_CXX_COMPILER g++)

# Set source file directory  
set(source_directory .)

# Find and add libraries and headers to build script                     
find_package(BOOST REQUIRED)
find_package(BLITZ REQUIRED)
find_package(NETCDF_CPP REQUIRED)      
find_package(GRAPHICSMAGICK REQUIRED)       

# Get sources
file(GLOB sources ${source_directory}/*.cpp)

# Explicitly add ImageMagick headers to build since it's doing something
# weird at the moment
set(INCLUDES /usr/local/include/GraphicsMagick)
set(LIBRARIES /usr/local/lib)

# Set up executable 
add_executable(${PROJECT_NAME} ${sources})
include_directories(${INCLUDES} 
    ${source_directory} 
    ${BOOST_INCLUDE_DIR} 
    ${BLITZ_INCLUDE_DIRS} 
    ${NETCDF_INCLUDE_DIRS} 
    ${GRAPHICSMAGICK_INCLUDE_DIRS}) 
target_link_libraries(${PROJECT_NAME} 
    ${BLITZ_LIBRARIES}
    ${BOOST_LIBRARIES}       
    ${GRAPHICSMAGICK_LIBRARIES} 
    ${NETCDF_CPP_LIBRARIES})
set_target_properties(${PROJECT_NAME} 
    PROPERTIES COMPILER_FLAGS "-fast -m64 -arch i386 -msse -Wall -pedantic"
               LINKER_FLAGS "-fast -m64 -arch i386 -msse")