/*
    common.hpp (ImageAnalyst)
    Jess Robertson, 2011-02-17
    
    Common includes for ImageAnalyst code
*/                                       

#ifndef COMMON_HPP_P0A71YOS
#define COMMON_HPP_P0A71YOS
     
#include <iostream>
#include <sstream>  
#include <fstream>
#include <string> 
#include <cstring> 
#include <vector> 
#include <set> 
#include <list>            
#include <algorithm> 
#include <math.h>
#include <GraphicsMagick/Magick++.h>
#include <blitz/array.h>      
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/tuple/tuple_comparison.hpp> 
#include <boost/foreach.hpp>   
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>                   
#include <boost/regex.hpp>  

namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;

#define BOOST_FILESYSTEM_NO_DEPRECATED 

#ifndef foreach
#define foreach BOOST_FOREACH
#endif
            
#endif /* end of include guard: COMMON_HPP_P0A71YOS */
