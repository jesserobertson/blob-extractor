# Find boost include directories and libraries
#
# BOOST_INCLUDE_DIRECTORIES - where to find netcdf.h
# BOOST_LIBRARIES - list of libraries to link against when using NetCDF
# BOOST_FOUND - Do not attempt to use NetCDF if "no", "0", or undefined.

include(LibFindMacros)

# Dependencies 
set(BOOST_PREFIX "/usr/local" 
    CACHE PATH "Path to search for boost header and library files" )

# Find include directories
find_path(BOOST_SYSTEM_INCLUDE_DIR
    NAMES boost/system/system_error.hpp PATHS ${BOOST_PREFIX})
find_path(BOOST_FILESYSTEM_INCLUDE_DIR 
    NAMES boost/filesystem.hpp PATHS ${BOOST_PREFIX})
find_path(BOOST_REGEX_INCLUDE_DIR 
    NAMES boost/regex.hpp PATHS ${BOOST_PREFIX})
find_path(BOOST_PROGRAM_OPTIONS_INCLUDE_DIR 
    NAMES boost/program_options.hpp PATHS ${BOOST_PREFIX}) 

# Finally the libraries themselves
find_library(BOOST_SYSTEM_LIBRARY
    NAMES boost_system boost_system-mt
    PATHS ${BOOST_PREFIX})
find_library(BOOST_FILESYSTEM_LIBRARY 
    NAMES boost_filesystem boost_filesystem-mt 
    PATHS ${BOOST_PREFIX})
find_library(BOOST_REGEX_LIBRARY 
    NAMES boost_regex boost_regex-mt 
    PATHS ${BOOST_PREFIX})
find_library(BOOST_PROGRAM_OPTIONS_LIBRARY 
    NAMES boost_program_options boost_program_options-mt 
    PATHS ${BOOST_PREFIX})

# Set the include dir variables and the libraries and let libfind_process do
# the rest. NOTE: Singular variables for this library, plural for libraries
# this this lib depends on.
set(BOOST_PROCESS_INCLUDES BOOST_SYSTEM_INCLUDE_DIR BOOST_FILESYSTEM_INCLUDE_DIR BOOST_REGEX_INCLUDE_DIR BOOST_PROGRAM_OPTIONS_INCLUDE_DIR)
set(BOOST_PROCESS_LIBS BOOST_SYSTEM_LIBRARY BOOST_FILESYSTEM_LIBRARY BOOST_REGEX_LIBRARY BOOST_PROGRAM_OPTIONS_LIBRARY)
libfind_process(BOOST)