/*
	analyst.hpp (ImageAnalyst)
	Jess Robertson, 2010-06-15
*/                            

#ifndef BLOB_HPP_4E6ZC9KD
#define BLOB_HPP_4E6ZC9KD   

#include "common.hpp"
#include "types.hpp"   
#include "utilities.hpp"                        
#include "logger.hpp"   

// = Settings struct =
typedef struct {                     
    blitz::TinyVector<int, 4> segmentWindow;
    double thresholdFraction;
    int blobSize;
    bool saveChangedFile;
} AnalystSettings;

// = Class interface =
class ImageAnalyst: public Magick::Image {
public:                              
	ImageAnalyst(const bfs::path fileLocation, 
	             const AnalystSettings& settings);
	virtual ~ImageAnalyst();   
	
	// Analysis methods    
	void segment(); 
	
	// Accessor methods - must call segment first
    void get_centroids(std::vector<Index>& centroids);                        
    Label get_maximum_label();
    void get_blob(Label label, std::vector<Index>& blob);
    
    inline blitz::TinyVector<int, 4> get_window_size() {
        blitz::TinyVector<int, 4> result(iMin, iMax, jMin, jMax);
        return result;
    };
	                                                   
private: 
    // Boundaries of segmentation window
    int iMin, iMax, jMin, jMax;
     
	// Data                               
	const bfs::path fileLocation; // image file location
	AnalystSettings settings;   
    
	// Segmentation data    
    blitz::Array<Label, 2> labelArray;                         
    std::list< std::set<Label> > equivalentLabels; 
    Index currentIndex;
    std::set<Label> neighbourValues;
    
    // Useful data once image has been segmented 
    static const Label background = 0;
    Label maxLabel;
    bool notSegmented; 
    std::vector< std::vector<Index> > labelLocations;
    
    // Segmentation functions  
    void _update_labels(int i, int j);
	
	// Logging
	const static LogLevel localLoggingLevel = traceLevel;   
	std::auto_ptr<Logger> logger;		   
}; 

// = Exceptions =
class ImageNotSegmented: public std::exception { 
public:
    virtual const char* what() const throw() { 
        std::ostringstream msg;
        msg << "Image hasn't been segmented before call to "
            << "ImageAnalyst::get_centroids";
        return msg.str().c_str();
    }
};
class InvalidLabel: public std::exception { 
public:
    InvalidLabel(Label label, Label max, Label background):
        _label(label), 
        _max(max), 
        _background(background) 
    { /* pass */ }
    virtual const char* what() const throw() {
        std::ostringstream msg;
        msg << "Label " << _label << " is not in valid range (" 
            << _background << " to " << _max << ")";
        return msg.str().c_str();
    }        
           
private:
    Label _label, _max, _background;
};                   

#endif
