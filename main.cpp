/*
    main.cpp (ImageAnalyst)
    Jess Robertson, 2011-02-16
*/  

#include "common.hpp"
#include "crawler.hpp"       
#include "logger.hpp" 

// Pattern for jpeg files
static const boost::regex jpegPattern("jpeg");

int main (int argc, char *argv[]) {
    // Set up logger instance
    std::auto_ptr<Logger> logger(new Logger(debugLevel));
    Magick::InitializeMagick(NULL);
    
    // Declare some options variables
    bool recurse = false, dump = false;  
    double thresholdFraction;
    int blobSize;
    bfs::path dumpFile = "dump.py";
    std::vector<bfs::path> directories;  
    std::string regex;
    
    // Set up variable descriptions
    bpo::options_description visible(\
        "Printing help message\n\nUsage: ./process_images [options] <files/paths>\n\nOptions");
    visible.add_options()                                               \
        ("help", "prints this help message")                            \
        ("regex", bpo::value(&regex),                                   \
         "provides a regular expression to match filenames against")    \
        ("recursive", "sets whether trees are traversed recursively")   \
        ("save-segments", "whether to save segmented image file")       \
        ("threshold", bpo::value<double>(&thresholdFraction),           \
         "sets thresholding fraction for blob extraction")              \
        ("window", bpo::value< std::vector<int> >()->multitoken(),      \
         "window from which blobs are extracted (=x1 x2 y1 y2)")        \
        ("size", bpo::value<int>(&blobSize),                            \
         "blob size (in pixels) to use for blob extraction")            \
        ("output", bpo::value<bfs::path>(&dumpFile),                    \
         "file into which program should dump data");
    bpo::options_description hidden("Hidden options");
    hidden.add_options()("search-path", \
        bpo::value< std::vector<bfs::path> >(&directories), "search path");
    
    // Compose options for command line 
    bpo::options_description cmdline_options;
    cmdline_options.add(visible).add(hidden);

    bpo::positional_options_description positional;
    positional.add("search-path", -1);  

    // Parse command line variables
    try {
        bpo::variables_map varMap;
        bpo::store(
            bpo::command_line_parser(argc, argv).\
                options(cmdline_options).positional(positional).run(),
            varMap);
        bpo::notify(varMap);
        
        // Check for help flag
        if (varMap.count("help")) {
            logger->message(visible, traceLevel);
            return 1;
        }                             
        
        // Traverse over supplied directories
        if (varMap.count("search-path")) {  
            // Set default crawler settings
            CrawlerSettings crawl_settings;
            crawl_settings.matchRegex = jpegPattern;
            crawl_settings.recursive = false;
            crawl_settings.output = false;
            crawl_settings.outputfile = "output.py";  
            
            // Set crawler settings from options
            if (varMap.count("regex")) 
                crawl_settings.matchRegex.assign(regex);
            if (varMap.count("recursive")) 
                crawl_settings.recursive = true; 
            if (varMap.count("output")) { 
                crawl_settings.output = true;
                crawl_settings.outputfile = dumpFile;  
            } 
            
            // Set default analyst settings
            AnalystSettings analyst_settings;
            analyst_settings.blobSize = 5;
            analyst_settings.thresholdFraction = 0.8;
            analyst_settings.saveChangedFile = false; 
            
            // Set window settings
            if (varMap.count("window")) {
                std::vector<int> values = varMap["window"].as< std::vector<int> >();
                if (values.size() == 4) {
                    for (int i=0; i<4; i++)
                        analyst_settings.segmentWindow(i) = values[i];
                } else {            
                    std::ostringstream msg;
                    msg << "Four integers needed for window specification"
                        << " (passed by --window)";
                    logger->message(msg.str(), errorLevel);
                    logger->message("Ignoring --window input", warningLevel);
                }
            } else {
                for (int i=0; i<4; i++)
                    analyst_settings.segmentWindow(i) = -1;
            } 
            
            // Set analyst settings from options
            if (varMap.count("threshold"))  
                analyst_settings.thresholdFraction = thresholdFraction;
            if (varMap.count("size"))            
                analyst_settings.blobSize = blobSize;  
            if (varMap.count("save-segments"))
                analyst_settings.saveChangedFile = true;
            
            Crawler crawler(crawl_settings, analyst_settings); 
            foreach(bfs::path p, directories) 
                crawler(p); 
        } else {
            throw InvalidDirectorySpec();
        }   
        return 0;  
    } catch(std::exception& e) {
        logger->message(e.what(), errorLevel);
        logger->message(visible, debugLevel);
        return 1;
    }

	return 0;
}