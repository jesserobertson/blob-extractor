/*
    crawler.cpp
    Jess Robertson, 2011-02-17
    
    Implementation of Crawler class methods
*/                                         

#include "crawler.hpp" 

// Ctor, dtor etc
Crawler::Crawler(const CrawlerSettings& s, const AnalystSettings& as):  
    settings(s), analyst_settings(as), logger(new Logger(localLoggingLevel))
{
    logger->message("Constructed crawler instance", debugLevel);
}
Crawler::~Crawler() {
    logger->message("Destructing crawler instance", debugLevel);
} 

// Operator for given path
void Crawler::operator()(const bfs::path& path) {
    // Check argument type: only directories or regex-matched files allowed
    if (bfs::is_directory(path)) {
        std::ostringstream msg;
        msg << "Traversing " << path;
        logger->message(msg.str(), traceLevel);
        
        for(bfs::directory_iterator it(path), end; it != end; it++) {
            // Traverse directory: First check whether path points to a directory, 
            // and call function recursively if recurse flag is set. Otherwise  
            // path points to a file, so call function recursively on it. 
            if (bfs::is_directory(it->path()) && not(settings.recursive)) {
                _ignore_message(it->path());
                continue;
            }
            (*this)(it->path());
        }    
    } else if (_match_regex(path.filename())) 
        analyse_image(path);
    else _ignore_message(path); 
}  

// Analysis routine
void Crawler::analyse_image(const bfs::path& path) 
{
    std::ostringstream msg;
    msg << "Running image analysis on " << path;
	logger->message(msg.str(), traceLevel);

    // Construct and segment picture
    std::auto_ptr<ImageAnalyst> \
        analyst(new ImageAnalyst(bfs::absolute(path.filename()), 
            analyst_settings));
	analyst->segment();
	
	// Get centroids and dump to file if required 
	if (settings.output) {   
	    std::ostringstream msg; 
        msg << "Dumping segment centroids to " << settings.outputfile;
        logger->message(msg.str(), traceLevel);
        
        // Get segmentation window size
        blitz::TinyVector<int, 4> wsize = analyst->get_window_size();
        
        // Open dumpfile as stream, add path, altered path and image and 
        // window sizes  
        std::fstream dumpFileStream;
        dumpFileStream.open(settings.outputfile.string().c_str(), 
            std::fstream::out | std::fstream::app); 
        dumpFileStream << "{'original_file': '" << path.string()
                       << "', 'segmented_file': '" 
                       << path.stem() << "_segments" << bfs::extension(path) 
                       << "', 'image_size': (" 
                       << analyst->columns() << ", " << analyst->rows() 
                       << "), 'window_size': (" << wsize[0] << ", " 
                       << wsize[1] << ", " << wsize[2] << ", " 
                       << wsize[3] << "), ";
        
        // Get centroids, push to file
        std::vector<Index> centroids;
        analyst->get_centroids(centroids); 
        dumpFileStream << "'centroids': [";
        foreach(Index index, centroids)
            dumpFileStream << "(" << index[0] << "," << index[1] << "), ";
        dumpFileStream << "]}" << std::endl;  
        
        // Clean up
        dumpFileStream.flush();
        dumpFileStream.close();
    }
    
    // Exit
	logger->message("Done!", traceLevel);
}