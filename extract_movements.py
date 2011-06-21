#!/usr/bin/env python
# encoding: utf-8
# Jess Robertson, 2011-02-20
#
# Contains PositionAnalyser class which takes the output from process_images and
# extracts trails from these.

import sys
import getopt                             
from numpy import sqrt, linspace, nan, transpose, vectorize, meshgrid
from matplotlib.pyplot import figure, subplot
from matplotlib.mlab import griddata       
from matplotlib.cm import RdYlGn as myColorMap 
from matplotlib.gridspec import GridSpec
              
# ==> Usage exception class and help message =================================

help_message = \
"""Usage: ./extract_movements [options] <centroid_file>\n
Options:
\t--output=<file>  File to which object trails should be dumped.
\t--help           Displays this message.
"""
class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg
    


# ==> Distance functions =====================================================

def euclidean_distance(x1, x2):
    """ Euclidean distance function between two supplied points.
         
        The Euclidean distance is simply ((x2-x1)**2 + (y2-y1)**2).
    """
    return (x2[0] - x1[0])**2 + (x2[1] - x1[1])**2

def directed_distance(trail, point):
    """ Distance function which uses trail information to determine distance.
        
        The expected location is just the end of the trail plus the difference 
        in position between the last and second last points in the trail, i.e.:
            
            expected = trail[-1] - (trail[-2] - trail[-1])
        
        The distance function calculates the Euclidean distance from this 
        expected location. This should be more accurate at finding the next 
        point in the tral when the motion of the trail is regular. 
    """
    expectedLocation = \
        (trail[-1][0] + (trail[-1][0] - trail[-2][0]),
         trail[-1][1] + (trail[-1][1] - trail[-2][1]))
    return euclidean_distance(expectedLocation, point)


# ==> Data Analysis class ==================================================== 

class PositionAnalyser(object):
    """ Analyses positions imported from a dump file from ./process_images.
        
        This uses the given location weighting function and a set of blob
        centroids extracted from a video feed by ./process_images to generate 
        a list of trails which track the motion of the blobs while they are in 
        the centre of the image. The settings below should be set to the 
        relevant sizes for the images given.
        
            _Gutter }   These parameters define the width of the boundaries of  
                        the image which are watched for the entry or exit of   
                        blobs. 
                        
             entry }    At each step, the program checks the gutters given in 
             exit  }    the entry array and creates new blobs if a centroid is 
                        found there. Similarly the program terminates a trail 
                        which enters a gutter marked exit.
    """
    def __init__(self, centroidsFile=None):
        super(PositionAnalyser, self).__init__()
        self.centroidsFile  = centroidsFile 
        self.liveTrails     = [] # <-- Trails which are actively being tracked
        self.completeTrails = [] # <-- Trails which entered one of the gutter 
                                 #     regions and were killed
        self.trails         = [] # <-- Contains all trails once trail extraction 
                                 #     is complete
        self.entryGutters = {'right': 50}
        self.exitGutters = {'left': 50} 
            
        centroidImageDict = eval(open(self.centroidsFile, 'r').readline())
        self.imageSize = centroidImageDict['image_size']
        self.windowSize = centroidImageDict['window_size']  
    
    def extract_trails(self): 
        """ Reads in the given dumpfile, parses the dictionaries of locations 
            for each frame and updates the trails with the new locations.
        """ 
        if self.centroidsFile is None: 
            msg = "No centroid file found!" + \
                  "\n\t For help use --help".format(self.centroidsFile)
            raise Usage(msg)
        try:
            for line in open(self.centroidsFile, 'r'):    
                self.__update_trails(line)
            
            # Copy out all extracted trails to self.trails attribute
            self.trails = self.completeTrails + self.liveTrails          
        except IOError, e: 
            msg = "Exception encountered when attempting " + \
                  "to read input file: {0}.\n\t -- Exception was: {1}" + \
                  "\n\t For help use --help".format(self.centroidsFile, e)
            raise Usage(msg)
    
    def read(self, inputFile):
        try:                    
            for line in open(inputFile, 'r'):    
                self.trails.append(eval(line))
        except IOError, e: 
            msg = "Exception encountered when attempting " + \
                  "to read input file: {0}.\n\t -- Exception was: {1}" + \
                  "\n\t For help use --help".format(self.centroidsFile, e)
            raise Usage(e)
    
    def write(self, outputFile): 
        
        """ Writes out the trails in self.trails to a dumpfile as a 
            Python list of tuples.
        """
                
        try:    
            f = open(outputFile + '.py', 'w')
            for trail in self.trails: 
                f.write("[")
                for index in trail:
                    f.write("({0}, {1}), ".format(*index)) 
                f.write("]\n")
                
        except IOError, e:
            msg = "Exception encountered when attempting " + \
                  "to write data to file: {0}." + \
                  "\n\t -- Exception was: {1}" + \
                  "\n\t For help use --help".format(outputFile, e)
            raise Usage(e) 
    
    def __update_trails(self, line):
        """ Updates trails with the positions in line. Utility function only 
            called from self.__extract_trails.
        """
        # Read in data from file - one line per image 
        if line[0] == '#': return
        centroidImageDict = eval(line)
        positions = centroidImageDict['centroids']  
        if len(positions) == 0: return
        
        # Iterate through trails, add nearest point and remove it 
        # from the possible positions. 
        added = []
        for trail in self.liveTrails:
            if len(trail) == 1:
                distances = [euclidean_distance(trail[0], p) \
                    for p in positions]    
                minIndex = distances.index(min(distances)) 
                trail.append(positions[minIndex])  
                added.append(positions[minIndex])  
            else:
                distances = [directed_distance(trail, p) \
                    for p in positions] 
                minIndex = distances.index(min(distances))
                trail.append(positions[minIndex]) 
                added.append(positions[minIndex])   
                
        # Add positions which are in the entry gutter as new trails 
        newPositions = \
            [ p for p in positions 
                if p not in added 
                if self.is_in_gutter(p, self.entryGutters)]    
        for p in newPositions:
                self.liveTrails.append([p])     
              
        # Remove trails which have entered a exit gutter zone
        toDelete = [] 
        for i, trail in enumerate(self.liveTrails):
            if self.is_in_gutter(trail[-1], self.exitGutters):
                toDelete.append(i) 
                if len(trail) > 1:
                    self.completeTrails.append(trail)
        toDelete.reverse()
        for i in toDelete:
            del self.liveTrails[i]
    
    def is_in_gutter(self, position, gutterDict): 
        """ Returns whether a position is within one of the given gutters.
        """
        for boundary, gutterSize in gutterDict.iteritems():
            if boundary == 'left': 
                return (position[0] < self.windowSize[0] + gutterSize)
            elif boundary == 'right': 
                return (position[0] > self.windowSize[1] - gutterSize)
            elif boundary == 'top':
                return (position[1] > self.windowSize[2] - gutterSize)
            elif boundary == 'bottom':
                return (position[1] < self.windowSize[3] - gutterSize)
    


# ==> Plotter class ==========================================================

class PositionPlotter(object):
    """ Plots velocity plots from data in given PositionAnalyser instance.
        
        Units are generated from the values given in the framesPerSecond and 
        metersPerPixel attributes - these can be directly accessed and changed 
        as required.
    """
    def __init__(self, positionAnalyserInstance):
        super(PositionPlotter, self).__init__() 
        # Information for calculations
        self.analyst = positionAnalyserInstance 
        self.secondsPerFrame = 1/10.0  
        self.millimetersPerPixel = 2.95 
        
        # Information for plotting
        self.nbins = 50 # Number of bins in interpolator for contour plots  
        self.minXVel = -10
        self.maxXVel = 0
        self.minYVel = -0.5
        self.maxYVel = 0.5
        
        # Calculate instantaneous velocity data 
        self.instantVelocityData = [] 
        nFrames = 5; # number of frames to calculate velocity over
        for trail in self.analyst.trails:
            for pt1, pt2 in zip(trail[:-nFrames], trail[nFrames:]):
                dt = float(nFrames*self.secondsPerFrame)
                dx = float((pt2[0] - pt1[0])*self.millimetersPerPixel)
                dy = float((pt2[1] - pt1[1])*self.millimetersPerPixel)
                vel = (dx/dt, dy/dt)
                okVelocities = [ True
                    # self.minXVel < vel[0], 
                    # vel[0] < self.maxXVel,
                    # self.minYVel < vel[1],
                    # vel[1] < self.maxYVel    
                ]
                if all(okVelocities): 
                    self.instantVelocityData.append((pt1, vel))
                
        # Put velocity data in useful form for plot
        self.xs = [ float(pos[0]*self.metersPerPixel) 
                           for pos, vel in self.instantVelocityData ]
        self.ys = [ float(pos[1]*self.metersPerPixel) 
                           for pos, vel in self.instantVelocityData ]
        self.us = [ vel[0] for pos, vel in self.instantVelocityData ]
        self.vs = [ vel[1] for pos, vel in self.instantVelocityData ]
        self.magVel = [ sqrt(vel[0]**2 + vel[1]**2) \
                           for pos, vel in self.instantVelocityData ]
    
    def map_gutters(self, output):
        self.figure = figure(figsize=(10,3))
        self.axes = self.figure.gca()
        def gutter_label(x, y):
            if self.analyst.is_in_gutter((x, y), self.analyst.entryGutters): return 1;
            elif self.analyst.is_in_gutter((x, y), self.analyst.exitGutters): return -1;
            else: return 0 
        
        xWindowLim = (self.analyst.windowSize[0], self.analyst.windowSize[1])
        yWindowLim = (self.analyst.windowSize[2], self.analyst.windowSize[3])
        xs = linspace(xWindowLim[0], xWindowLim[1], self.nbins)
        ys = linspace(yWindowLim[0], yWindowLim[1], self.nbins)    
        xGrid, yGrid = meshgrid(xs, ys)
        zs = vectorize(gutter_label)(xGrid, yGrid) 
        csf = self.axes.contourf(xs, ys, zs, cmap=myColorMap)
        cbar = self.figure.colorbar(csf) 
        self.axes.set_aspect('equal')
        self.axes.set_xlim(*xWindowLim)
        self.axes.set_ylim(*yWindowLim)
        self.figure.savefig(output + '_gutters.pdf')
    
    def velocity_map(self, output='test'):
        """ Contours instantaneous velocity given by the analyser instance 
            passed to the plotting class.
        """
        self.figure = figure(figsize=(10,3))
        self.axes = self.figure.gca()   
        xWindowLim = (self.analyst.windowSize[0], self.analyst.windowSize[1])
        yWindowLim = (self.analyst.windowSize[2], self.analyst.windowSize[3])
        
        # Generate contours for velocity magnitude        
        xGrid   = linspace(\
            xWindowLim[0]*self.millimetersPerPixel, 
            xWindowLim[1]*self.millimetersPerPixel, self.nbins)
        yGrid   = linspace(\
            yWindowLim[0]*self.millimetersPerPixel, 
            yWindowLim[1]*self.millimetersPerPixel, self.nbins)
        magVelGrid = griddata(self.xs, self.ys, self.magVel, xGrid, yGrid)  
        # csf = self.axes.contourf(xGrid, yGrid, magVelGrid, range(2,26,2), cmap=myColorMap)
        csf = self.axes.contourf(xGrid, yGrid, magVelGrid, cmap=myColorMap)
        cbar = self.figure.colorbar(csf) 
        cbar.set_label("Velocity magnitude, px/s")
        
        # Generate arrow plot
        # q = self.axes.quiver(self.xs, self.ys, self.us, self.vs,
            # angles = 'xy', scale_units='xy', scale=2, pivot = 'mid')
        # self.axes.quiverkey(q, 0.9, 1.0, 10, "10 px/frame", coordinates='axes') 
        
        # Save figure  
        self.axes.set_aspect('equal')
        self.axes.set_xlim(*xWindowLim)
        self.axes.set_ylim(*yWindowLim)
        self.figure.savefig(output + '_velocity_map.pdf')       
    
    def velocity_curve(self, output='test'):
        """ Collapses all velocity data into a plot of velocity magnitude along 
            a given axes ('y' or 'x').
        """
        self.figure = figure() 
        self.gridSpec = GridSpec(2, 1)
        self.axes = subplot(self.gridSpec[0, 0])  
        self.axes.plot(self.xs, [-v for v in self.vs], 'ko', alpha=0.5) 
        self.axes.set_aspect('auto')
        self.axes = subplot(self.gridSpec[0, 1])  
        self.axes.plot(self.ys, [-u for u in self.us], 'ko', alpha=0.5) 
        self.axes.set_aspect('auto')
        self.figure.savefig(output + '_velocity_curve.pdf')
    


# ==> Main routine =========================================================== 

def main():                
    try:
        root = 'seqA'
        outputRoot = root + 'Trails'
        analyser = PositionAnalyser(root + 'Centroids.py') 
        analyser.extract_trails()
        plotter = PositionPlotter(analyser)  
        plotter.velocity_map(outputRoot) 
        plotter.velocity_curve(outputRoot)
        analyser.write(outputRoot)
        
    except Usage, e:
        print e.msg 
        
    

if __name__ == "__main__": main()