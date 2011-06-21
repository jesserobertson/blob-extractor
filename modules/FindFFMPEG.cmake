# Find FFMPEG include directories
#
# FFMPEG_INCLUDE_DIRECTORIES - where to find ffmpeg headers
# FFMPEG_FOUND - Do not attempt to use FFMPEG if "no", "0", or undefined.
    
include(LibFindMacros)

# Dependencies 
set(FFMPEG_PREFIX "/usr/local" 
    CACHE PATH "Path to search for FFMPEG header and library files" ) 

# Include dir
find_path(FFMPEG_INCLUDE_DIR 
    NAMES libavcodec/avcodec.h 
    PATHS ${BLITZ_PREFIX})   

# Finally the library itself
find_library(AVCODEC_LIBRARY 
    NAMES avcodec 
    PATHS ${FFMPEG_PREFIX})
find_library(AVFORMAT_LIBRARY 
    NAMES avformat 
    PATHS ${FFMPEG_PREFIX})
find_library(AVUTIL_LIBRARY 
    NAMES avutil 
    PATHS ${FFMPEG_PREFIX})

# Set the include dir variables and the libraries and let libfind_process do
# the rest. NOTE: Singular variables for this library, plural for libraries
# this this lib depends on.
set(FFMPEG_PROCESS_INCLUDES FFMPEG_INCLUDE_DIR)
set(FFMPEG_PROCESS_LIBS AVCODEC_LIBRARY AVFORMAT_LIBRARY AVUTIL_LIBRARY)
libfind_process(FFMPEG)

