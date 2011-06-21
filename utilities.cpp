/*
    utilites.cpp (ImageAnalyst)
    Jess Robertson, 2011-02-17
    
    Utility functions for Analyst
*/    

#include "utilities.hpp"       

/* insert_and_merge
   Inserts a new set into a list of sets, but merges any sets 
   which now have a non-empty intersection with the addition 
   of the new set. This maintains a completely disjuctive set of sets in 
   listOfSets. 
   
   We do this by looping over listOfSets and checking the intersection of 
   the new set with a given set. 
   -- If it's empty, then the labels in the two sets are not 
      equivalent (as far as we know) so we continue to the next 
      set in the list.
   -- If it's not empty then all labels in both sets are 
      equivalent, so we remove the tested set from the list, and 
      form the union of this with the current labels.
*/
void insert_and_merge(std::set<Label>& newSet, 
    std::list< std::set<Label> >& listOfSets) 
{
    // Every time we make changes we want to re-iterate to capture all 
    // equivalent sets in list.
    bool madeChanges = true;
    while (madeChanges) {
        madeChanges = false; 
        // Iterate through listOfSets, checking for intersection with newSet
        std::list< std::set<Label> >::iterator listIter = listOfSets.begin(); 
        while (listIter != listOfSets.end()) {
            bool intersectionExists = do_sets_intersect(\
               newSet.begin(), newSet.end(), 
               listIter->begin(), listIter->end()); 
            if (intersectionExists) { 
               // If intersection is not empty, remove current set from the 
               // list and merge it with the newSet. Note that list::erase
               // returns an incremented pointer.
               std::set<Label>::const_iterator setIter = listIter->begin();
               while (setIter != listIter->end())
                   newSet.insert(*setIter++);
               listIter = listOfSets.erase(listIter);
               madeChanges = true;
            } else {
               // Otherwise increment the list iterator
               listIter++;
            }
        }         
    }

    // Add newly merged set back into list
    listOfSets.push_front(newSet);                                           
}
