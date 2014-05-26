//
//  dft.h
//  example
//
//  Created by Grant Kot on 11/17/13.
//
//

#ifndef __example__dft__
#define __example__dft__

#include <iostream>

#define BANDS 100
#define MIN_FREQ .5
#define MAX_FREQ 50
class DFT {
    float lastTime;
    float sinAvg[BANDS], cosAvg[BANDS];
    float x[BANDS];
public:
    float amp[BANDS];
    DFT();
    void addPoint(float value, float time);
};

#endif /* defined(__example__dft__) */
