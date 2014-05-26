//
//  dft.cpp
//  example
//
//  Created by Grant Kot on 11/17/13.
//
//

#include "dft.h"
#include <math.h>
DFT::DFT() {
    lastTime = 0;
    
    for (int i = 0; i < BANDS; i++) {
        x[i] = 0;
        sinAvg[i] = 0;
        cosAvg[i] = 0;
    }
}

void DFT::addPoint(float value, float time) {
    float dt = 1.0/250.0;
    lastTime = time;
    
    for (int i = 0; i < BANDS; i++) {
        float frequency = (MIN_FREQ + (float)(MAX_FREQ-MIN_FREQ)*i/BANDS)*dt;
        x[i] = fmodf(x[i]+frequency*2*M_PI, 2*M_PI);
        sinAvg[i] += (sinf(x[i])*value - sinAvg[i])*.001;
        cosAvg[i] += (cosf(x[i])*value - cosAvg[i])*.001;
        amp[i] = sqrtf(sinAvg[i]*sinAvg[i]+cosAvg[i]*cosAvg[i]);
    }
}