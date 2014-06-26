//
//  ofxInlineFilter.h
//  barbicanExhibit
//
//  Created by Dan Goodwin on 6/25/14.
//
//


#include "ofMain.h"

#define NZEROS 4
#define NPOLES 4

    
class ofxInlineFilter {
    enum filterType {
        ALPHA_FILTER=0,
        BETA_FILTER=1
    };
    
public:

    float dGain;
    double xv[NZEROS+1]; float yv[NPOLES+1];
    
    ofxInlineFilter();
    
    float update(float input);
};