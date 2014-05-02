//
//  processing_math.h
//  emptyExample
//
//  Created by Daniel Goodwin on 5/2/14.
//
//

#ifndef __emptyExample__processing_math__
#define __emptyExample__processing_math__

#include <iostream>

#endif /* defined(__emptyExample__processing_math__) */

#include <string>
#include <math.h>

class StdDeviation
{
    
private:
    int max;
    double mean;
    vector<float> value;
public:
    
    double CalculateMean()
    {
        double sum = 0;
        for(int i = 0; i < max; i++)
            sum += value[i];
        return (sum / max);
    }
    
    double CalculateVariane()
    {
        mean = CalculateMean();
        
        double temp = 0;
        for(int i = 0; i < max; i++)
        {
            temp += (value[i] - mean) * (value[i] - mean) ;
        }
        return temp / max;
    }
    
    double CalculateSampleVariane()
    {
        mean = CalculateMean();
        
        double temp = 0;
        for(int i = 0; i < max; i++)
        {
            temp += (value[i] - mean) * (value[i] - mean) ;
        }
        return temp / (max - 1);
    }
    
    int SetValues(double *p, int count)
    {
        if(count > 100)
            return -1;
        max = count;
        for(int i = 0; i < count; i++)
            value[i] = p[i];
        return 0;
    }
    
    double GetStandardDeviation()
    {
        return sqrt(CalculateVariane());
    }
    
    double GetSampleStandardDeviation()
    {
        return sqrt(CalculateSampleVariane());
    }
    
};

int main()
{
    
    StdDeviation sd;
    
    sd.SetValues(arrNumbers, sizeof(arrNumbers) / sizeof(arrNumbers[0]));
    
    double mean = sd.CalculateMean();
    double variance = sd.CalculateVariane();
    double samplevariance = sd.CalculateSampleVariane();
    double sampledevi = sd.GetSampleStandardDeviation();
    double devi = sd.GetStandardDeviation();
    
    
    return 0;
}