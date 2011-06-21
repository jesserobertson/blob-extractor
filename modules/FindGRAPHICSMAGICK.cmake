# Find ImageMagick include directories and libraries
#
# GRAPHICSMAGICK_INCLUDE_DIRECTORIES - where to find ImageMagick headers
# GRAPHICSMAGICK_FOUND - Do not attempt to use ImageMagick if "no", "0", 
#                     or undefined. 
#

include(LibFindMacros)

# Dependencies 
set(GRAPHICSMAGICK_PREFIX "/usr/local/" 
    CACHE PATH "Path to search for imagemagick header and library files" )

# Find include directories
find_path(GRAPHICSMAGICK_INCLUDE_DIR
    NAMES GraphicsMagick/Magick++.h 
    PATHS ${GRAPHICSMAGICK_PREFIX}) 
    
# Finally the libraries themselves
find_library(GRAPHICSMAGICK_LIBRARY 
    NAMES GraphicsMagick++
    PATHS ${GRAPHICSMAGICK_PREFIX})
find_library(JASPER_LIBRARY NAMES jasper)
find_library(JPEG_LIBRARY NAMES jpeg) 

# Set the include dir variables and the libraries and let libfind_process do
# the rest. NOTE: Singular variables for this library, plural for libraries
# this this lib depends on.
set(GRAPHICSMAGICK_PROCESS_INCLUDES GRAPHICSMAGICK_INCLUDE_DIR)
set(GRAPHICSMAGICK_PROCESS_LIBS GRAPHICSMAGICK_LIBRARY JASPER_LIBRARY JPEG_LIBRARY)
libfind_process(GRAPHICSMAGICK)