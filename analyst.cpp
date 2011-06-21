/*
	analyst.cpp (ImageAnalyst)
	Jess Robertson, 2010-06-14
	
	This program uses ImageMagick to generate a thesholded image,
	segment it into blobs, and return their centroids.
*/                                  

#include "analyst.hpp"    

// Redefine BOOST_FOREACH so it doesn't look so ugly
#ifndef foreach
#define foreach BOOST_FOREACH
#endif     

// Construct/destruct etc
ImageAnalyst::ImageAnalyst(const bfs::path f, const AnalystSettings& s): 
    Image::Image(f.c_str()), fileLocation(f), notSegmented(true), 
    settings(s), logger(new Logger(localLoggingLevel)) 
{
	// Set window arguments  
    iMin = s.segmentWindow(0);
    iMax = std::min(s.segmentWindow(1), int(columns()));
    jMin = s.segmentWindow(2);
    jMax = std::min(s.segmentWindow(3), int(rows())); 
    if (iMin < 0) iMin = 0;
    if (jMin < 0) jMin = 0;
    if (iMax < 0) iMax = int(columns());
    if (jMax < 0) jMax = int(rows());
	
	logger->message("Constructed analyst instance", debugLevel);
}                                                  
ImageAnalyst::~ImageAnalyst() {
	logger->message("Deleting analyst instance", debugLevel);    
}                                

// = Segmentation implementation =
/*  Main method in the analyst class which extracts blobs from an image.
    The extraction proceeds as follows:
    1. Prepare image:  
        -- Boost constrast in the image
        -- Blur the image on the blob radius to reduce noise
        -- Reduce image to quantized greyscale 
       This should improve the convergence of the segmentation 
       algorithm by reducing the noise and making the differences 
       between image segments larger.  
    2. Segmentation sweep:
   	    1. Loop until a black pixel is found. 
   		2. If a black pixel is found then check its neighbours (whose indices 
   		   are less than the current index) for labels.
   			-- If one neighbour has a label, or more than one 
   			   neighbour has a label but these are the same, 
   			   assign the current pixel to that label by appending 
   			   its indices to the label vector and updating the label
   			   in the label image.
   			-- If more than one neighbour has a label 
   			   and they are different, add the two labels to the equivalent
   			   labels list, append the current pixel to the lowest label.
   			-- If no neighbours have labels then start a new label number 
   			   and append the current pixel location to it.              
           Then if there is more than one unique label value amongst a pixel's 
           labels, we insert and merge the labels with the equivalent labels 
           list. This is done by insert_and_merge so that the equivalent 
           labels list maintains a completely disjunctive set of sets.
   		   Step 2 is carried out by the ImageAnalyst::_update_labels method
   		3. Return to loop until next black pixel is found
*/
void ImageAnalyst::segment() {
    // Prepare image (using Magick++::Image methods)  
    blur(settings.blobSize);   
    quantizeDither(false); 
    quantizeColorSpace(Magick::GRAYColorspace); 
    quantize(); 
    threshold(settings.thresholdFraction*MaxRGB);       
    negate(); // Sets background = 0 
	
	// Generate labels image       
    logger->message("Making initial pass of image", debugLevel);
	labelArray.resize(columns(), rows()); 
    labelArray = background; 
    for(int i = iMin; i < iMax; ++i)
        for(int j = jMin; j < jMax; ++j)
            if (int(pixelColor(i, j).redQuantum()) != background)
                _update_labels(i, j);
    
    // Mark all equivalent labels in label array with the same number, these 
    // have already been stored in the labelLocationArray so just loop over 
    // that. Can use the stored indices for indirection in blitz array so 
    // can just pass the entire vector to the array. Note that blitz requires
    // another array for efficient indirection.      
    logger->message("Merging equivalent labels", debugLevel);
    Label currentLabelValue = 1;
    std::vector< std::vector<Index> > mergedLocations;             
    foreach(std::set<Label> labelSet, equivalentLabels) {
        // Merge all indices with equivalent labels into one index vector
        std::vector<Index> currentMergedIndices;
        foreach(Label label, labelSet)
            foreach(Index index, labelLocations[label-1])
                currentMergedIndices.push_back(index); 
        mergedLocations.push_back(currentMergedIndices);
                
        // Update values of specified pixels  
        blitz::Array<Label, 2> floodArray(columns(), rows());
        floodArray = currentLabelValue;
        labelArray[currentMergedIndices] = floodArray;
        
        // Get next label
        currentLabelValue++; 
    } 
    
    // Update new maximum label and location vectors  
    labelLocations = mergedLocations;
    maxLabel = currentLabelValue;   
    
    // Update segmentation flag to say that image has been segmented
    notSegmented = false;
    
    // If required, save segmented picture to file
    if (settings.saveChangedFile) {   
        // Replace array with current labelArray, with labels normalised 
        // by value to MaxRGB
        for(int i = iMin; i < iMax; ++i)
            for(int j = jMin; j < jMax; ++j) {
                int val = round(labelArray(i, j)*MaxRGB/maxLabel);
                pixelColor(i, j, Magick::Color(val, val, val));
            }    
        
        // Return colors to normal
        negate();                 
        
        // Add red dots for segment locations 
        std::vector<Index> centroids;
        get_centroids(centroids);     
        strokeColor("red");
        fillColor("none");
        strokeWidth(1);
        foreach(Index index, centroids)
            draw(Magick::DrawableCircle(
                index[0]-1, index[1]-1, index[0]+1, index[1]+1));  
                
        // Add red line for segment extraction boundary         
        draw(Magick::DrawableRectangle(iMin, jMin, iMax, jMax));
        
        // Check for segments folder, add if it doesn't exist 
        bfs::path segmentFolder = "segments";
        if (not(bfs::is_directory(segmentFolder)))
            bfs::create_directory(segmentFolder);
        
        // Write changed image to new file
        std::ostringstream segmentFile;
        segmentFile << segmentFolder.c_str() << "/"
                    << fileLocation.stem().c_str() 
                    << "_segments" << bfs::extension(fileLocation); 
        write(segmentFile.str().c_str());
    }   
}
void ImageAnalyst::_update_labels(int i, int j) {
    // Get the _non-zero_ label values of the west, northwest, northern and 
    // northeastern  pixels, and stash them in neighbourValues. 
    blitz::TinyVector<Label, 4> neighbours(0);
    if (i != 0 && j != 0)     neighbours[0] = labelArray(i-1, j-1);
    if (i != 0)               neighbours[1] = labelArray(i-1, j);
    if (j != 0)               neighbours[2] = labelArray(i, j-1);
    if (i != columns()-1 && j != 0) neighbours[3] = labelArray(i+1, j-1);
    neighbourValues.clear();  
    foreach(Label label, neighbours)
        if (label > 0) 
            neighbourValues.insert(label);      
    
    // Check the neighbours' values, perform relevant updates  
    Index index(i, j);
    if (neighbourValues.size() > 0) {
        // Label a point in the label array with the maximum label of it's 
        // neighbours, and add the current label location to the list of label 
        // locations in the labelLocationList attribute
        Label currentLabel = \
            *(std::max_element(
                neighbourValues.begin(), 
                neighbourValues.end()));
        labelArray(index) = currentLabel;   
        labelLocations[currentLabel-1].push_back(index);  
           
        // Record that multiple neighbour labels are equivalent, if there's 
        // more than one value in the array.
        if (neighbourValues.size() > 1) 
            insert_and_merge(neighbourValues, equivalentLabels);
    } else {
        // There are no labelled neighbours, so make a new label for the 
        // current index which is higher than all previous labels 
        std::vector<Index> newLabelVec(1, index);
        labelLocations.push_back(newLabelVec); 
        labelArray(index) = labelLocations.size();  
    }
} 

// = Accessor methods for blob data =
void ImageAnalyst::get_centroids(std::vector<Index>& centroids) {
    // Check that image has already been segmented
    if (notSegmented) throw ImageNotSegmented();
    
    // Otherwise, return the location of the blob centroids by summing the 
    // indices of pixels in the blob and returning their mean. 
    foreach(std::vector<Index> blob, labelLocations) {  
        Index currentCentroid(0, 0);
        foreach(Index index, blob)
            currentCentroid += index; // Store sums in centroid
        currentCentroid[0] = int(currentCentroid[0]/double(blob.size()));
        currentCentroid[1] = int(currentCentroid[1]/double(blob.size()));
        centroids.push_back(currentCentroid);
    }
}
Label ImageAnalyst::get_maximum_label() {
    if (notSegmented) throw ImageNotSegmented();
    return maxLabel;
}  
void ImageAnalyst::get_blob(Label label, std::vector<Index>& blob) {
    if (notSegmented) throw ImageNotSegmented();
    if (label > maxLabel || label < background) 
        throw InvalidLabel(label, maxLabel, background);
    blob = labelLocations[label-1];
} 