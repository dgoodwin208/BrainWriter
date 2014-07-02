//
//  ofxInlineFilter.cpp
//  barbicanExhibit
//
//  Created by Dan Goodwin on 6/25/14.
//
//

#include "ofxInlineFilter.h"
#include <math.h>

//Generating an online filter, using the script at http://www-users.cs.york.ac.uk/~fisher/mkfilter
//And the sample code from this SO article http://stackoverflow.com/questions/664877/i-need-to-implement-a-butterworth-filter-in-c-is-it-easier-get-a-library-with-t
ofxInlineFilter::ofxInlineFilter()
{
    isSetup = false;
  
}

void ofxInlineFilter::setup(float nPoles, float samplingRate, float freqLowBand, float freqHighBand){
    //inputs:
    n  = nPoles; //order of filter
    s = samplingRate; //sampling frequency
    f1 = freqHighBand; //cutoff high
    f2 = freqLowBand; //cutoff low
    
    double a = cos(M_PI*(f1+f2)/s)/cos(M_PI*(f1-f2)/s);
    double a2 = a*a;
    double b = tan(M_PI*(f1-f2)/s);
    double b2 = b*b;
    double r;
    
    n = n/4;
    
    w0 = (double *)calloc(n, sizeof(double));
    w1 = (double *)calloc(n, sizeof(double));
    w2 = (double *)calloc(n, sizeof(double));
    w3 = (double *)calloc(n, sizeof(double));
    w4 = (double *)calloc(n, sizeof(double));
    
    d1 = (double *)malloc(n*sizeof(double));
    d2 = (double *)malloc(n*sizeof(double));
    d3 = (double *)malloc(n*sizeof(double));
    d4 = (double *)malloc(n*sizeof(double));
    
    A = (double *)malloc(n*sizeof(double));
    
    for(int i=0; i<n; ++i){
        r = sin(M_PI*(2.0*i+1.0)/(4.0*n));
        s = b2 + 2.0*b*r + 1.0;
        A[i] = b2/s;
        d1[i] = 4.0*a*(1.0+b*r)/s;
        d2[i] = 2.0*(b2-2.0*a2-1.0)/s;
        d3[i] = 4.0*a*(1.0-b*r)/s;
        d4[i] = -(b2 - 2.0*b*r + 1.0)/s;
    }
    printf("Setup filter\n");
    isSetup = true;
}

double ofxInlineFilter::update(float input){
    if (!isSetup) {
        printf("ERROR: Need to call setup() to initialize this filter");
        return -1;
    }
    double x;
    
    for(int i=0; i<n; ++i){
        w0[i] = d1[i]*w1[i] + d2[i]*w2[i]+ d3[i]*w3[i]+ d4[i]*w4[i] + (double)input;
        x = A[i]*(w0[i] - 2.0*w2[i] + w4[i]);
        w4[i] = w3[i];
        w3[i] = w2[i];
        w2[i] = w1[i];
        w1[i] = w0[i];
    }

    return x;
}

