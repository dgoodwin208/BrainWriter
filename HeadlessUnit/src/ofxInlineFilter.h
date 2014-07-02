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
    
public:

    ofxInlineFilter();
    double update(float input);
    void setup(float nPoles, float samplingRate, float freqLowBand, float freqHighBand);

    
    bool isSetup;
    
    
    //This filter code was taken from http://www.exstrom.com/journal/sigproc/bwbpf.c
    //under GNU public license. (Thank you!)
    
    double n; //order of filter
    double s; //sampling frequency
    double f1; //cutoff low
    double f2; //cutoff high
    
    double *w0;
    double *w1;
    double *w2;
    double *w3;
    double *w4;
    double *d1;
    double *d2;
    double *d3;
    double *d4;
    
    double *A;

    
    float dGain;
    double xv[NZEROS+1]; float yv[NPOLES+1];
    
};