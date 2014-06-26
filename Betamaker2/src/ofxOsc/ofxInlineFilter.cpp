//
//  ofxInlineFilter.cpp
//  barbicanExhibit
//
//  Created by Dan Goodwin on 6/25/14.
//
//

#include "ofxInlineFilter.h"


//Generating an online filter, using the script at http://www-users.cs.york.ac.uk/~fisher/mkfilter
//And the sample code from this SO article http://stackoverflow.com/questions/664877/i-need-to-implement-a-butterworth-filter-in-c-is-it-easier-get-a-library-with-t
ofxInlineFilter::ofxInlineFilter()
{
//    switch (fType) {
//        case ALPHA_FILTER:
//            cout << "test";
//            initParam=6;
//            dCoefficient[0] = 0;
//            dCoefficient[1] = 0;
//            dCoefficient[2] = 0;
//            dCoefficient[3] = 0;
//            dCoefficient[4] = 0;
//            break;
//        case BETA_FILTER:
//            dCoefficient[0] = 0;
//            dCoefficient[1] = 0;
//            dCoefficient[2] = 0;
//            dCoefficient[3] = 0;
//            dCoefficient[4] = 0;
//
//            break;;
//        default:
//            break;
//
//            dGain =  1.401048790e+06;
//            
//              }
    //initialize the array
    printf("Initialized filter\n");
    for (int i=0; i<NPOLES+1; ++i) {
        xv[i]=0.; yv[i]=0.;
    }
    //dGain =  1.401048790e+06;
}



float ofxInlineFilter::update(float input){
    
    dGain = 3.079544182e+06;
    
    xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4];

    xv[4] = (input)*dGain;
    yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4];
    yv[4] =   (xv[0] + xv[4]) - 2 * xv[2] + ( -0.9983893681 * yv[0]) + (  3.9951625486 * yv[1]) + ( -5.9951569893 * yv[2]) + (  3.9983838089 * yv[3]);

    printf("FILTER STATUS\n");
    for (int i=0; i<NPOLES+1; ++i) {
        printf("%f\t%f\n",xv[i],yv[i]);
    }
    
    return (yv[4]);
}