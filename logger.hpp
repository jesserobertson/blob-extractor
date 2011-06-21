/*
	logger.hpp (ImageAnalyst)
	Jess Robertson, 2010-06-15
	
	Implemetation of a simple logger class. A logger may be initialised
	within a given class with a logging level. The logger will only print 
	messages with levels larger than the given logging level.
*/              

#ifndef LOGGER_HPP_3LP07D20
#define LOGGER_HPP_3LP07D20   

#include "common.hpp"
   
// Define logging levels in order of precedence      
enum LogLevel {
    errorLevel, 
    warningLevel, 
    traceLevel, 
    debugLevel, 
    allLevels
};

// Interface and implementation of logger class
class Logger {
public:  
	// Construct, destruct...
	Logger(LogLevel level=warningLevel) {
		logLevel = level; 
		message("Constructed Logger instance", debugLevel);
	};
	virtual ~Logger() {
		message("Destructing Logger instance", debugLevel);
	};           
	
	// Write method
	template<typename T> inline void message(T msg, LogLevel level) {
         if (level <= logLevel)
			std::cout << _prefix(level) << msg << std::endl;
	}

private:
	// Data
	LogLevel logLevel;
	
	// Private methods
	inline std::string _prefix(LogLevel level) { 
		// Provides a prefix string depending on the level of the 
		// logger message
		std::stringstream buffer;		
		switch (level) {       
			case errorLevel:   buffer << "**ERROR** "; break;
			case warningLevel: buffer << " Warning: "; break;
			case traceLevel:   buffer << "      --> "; break;  
			case debugLevel:   
			case allLevels:  
			default:           buffer << "          "; break;
		}
		return buffer.str();
	}  
}; 

#endif /* end of include guard: LOGGER_HPP_3LP07D20 */
