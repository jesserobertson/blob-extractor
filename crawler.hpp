/*
    crawler.hpp (ImageAnalyst)
    Jess Robertson, 2011-02-17
    
    Simple data crawler to make it easier to alter a number of image files.
*/      

#ifndef CRAWLER_HPP_K7OISOXZ
#define CRAWLER_HPP_K7OISOXZ  

#include "common.hpp"
#include "logger.hpp"
#include "analyst.hpp"  

typedef struct {
    boost::regex matchRegex; 
    bool recursive; 
    bool output; 
    bfs::path outputfile;
} CrawlerSettings;

// = Class interface =
class Crawler {
public: 
    Crawler(const CrawlerSettings& s, const AnalystSettings& as);
    virtual ~Crawler();      
    void operator()(const bfs::path& p);     
    void analyse_image(const bfs::path& f);
    
private:   
    const CrawlerSettings settings;
    const AnalystSettings analyst_settings;
    
    // Private methods    
    inline bool _match_regex(bfs::path path) {
        return boost::regex_search(to_string(path), settings.matchRegex);
    }
    inline void _ignore_message(const bfs::path& path) {
        std::ostringstream msg;
        msg << "Ignored " << path;
        logger->message(msg.str(), traceLevel);
    }
    inline void _change_message(const bfs::path& path) {
        std::ostringstream msg;
        msg << "Changed " << path;
        logger->message(msg.str(), traceLevel);
    }
    inline void _dump_message(const bfs::path& path) {
        std::ostringstream msg;
        msg << "Dumping " << path << " to output file " << settings.outputfile;
        logger->message(msg.str(), traceLevel);
    }
    
    // Logging
	const static LogLevel localLoggingLevel = traceLevel;   
	std::auto_ptr<Logger> logger;   
};

// = Exceptions =
class InvalidDirectorySpec: public std::exception {
    virtual const char* what() const throw() { 
        std::ostringstream msg;
        msg << "No directory specified for program execution";
        return msg.str().c_str();
    }
}; 
class InvalidPath: public std::exception {
public:
    InvalidPath(bfs::path p): _path(p) { /* pass */ } 
    virtual ~InvalidPath() throw() { /* pass */ }
    virtual const char* what() const throw() { 
        std::ostringstream msg;
        msg << "Invalid path specification: " << _path;
        return msg.str().c_str();
    }
private:
    const bfs::path _path;
};

#endif /* end of include guard: CRAWLER_HPP_K7OISOXZ */
