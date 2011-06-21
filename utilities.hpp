/*
    utilities.hpp (ImageAnalyst)
    Jess Robertson, 2011-02-17
    
    Some helper functions for Image Analyst
*/

#ifndef UTILITIES_HPP_2S3C0BGX
#define UTILITIES_HPP_2S3C0BGX  

#include "common.hpp"
#include "types.hpp"

/* make_str template to simplify printing values with << operators
   First template is for classes with a << operator, second one is for  
   containers which contain classes with a << operator.
*/
template<class T> 
inline std::string to_string(const T& value) {
    std::ostringstream streamOut;
    streamOut << value;
    return streamOut.str();
}
template <class Iterator> 
inline std::string to_string(Iterator inputBegin, Iterator inputEnd) { 
    std::ostringstream streamOut;
    while (inputBegin != inputEnd) 
        streamOut << *(inputBegin++) << ", ";
    streamOut << std::endl;
    return streamOut.str();
}
  
/* do_sets_intersect 
   Template determines whether the supplied ordered containers have a 
   (non-empty) intersection. Algorithm uses the fact that sets must be 
   ordered. This loops through the two sets using iterators and at each step 
   it compares the values.       
    -- If the iterator values are the same then we are done.
    -- If either iterator is larger than the other then increment the 
       iterator with the smaller value, since the ordering means the smaller 
       iterator value cannot be in the set.                                           
   If no intersection is found (i.e. when one of the sequences terminates) 
   this function will return false. 
*/ 
template <class IterA, class IterB>                               
bool do_sets_intersect(IterA A, IterA endA, IterB B, IterB endB) { 
    bool intersectQ = false;
    while ((A != endA) && (B != endB)) {
        if (*A<*B) ++A; 
        else if (*B<*A) ++B;
        else { intersectQ = true; break; }
    }  
    return intersectQ;
} 

void insert_and_merge(std::set<Label>& newSet, 
    std::list< std::set<Label> >& listOfSets);

#endif /* end of include guard: UTILITIES_HPP_2S3C0BGX */
