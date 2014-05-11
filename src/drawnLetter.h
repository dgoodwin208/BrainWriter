//
//  drawnLetter.h
//  emptyExample
//
//  Created by Daniel Goodwin on 5/10/14.
//
//

#include <iostream>
#include "ofMain.h"



//Control parameters class
class DrawnLetter {
public:
    DrawnLetter();
    void setup();
    void deleteLast();
    void draw(int mouse_x, int mouse_y);
    void closeShape();
    void update();
    bool handleClick(ofPoint clickPos);
    void showSampleLine(ofPoint mousePos);
    void makeBend(int idx, ofPoint mousePos);
    void setLineBend(int lineIdx);
    
	ofPolyline createLineSegment(ofPoint a, ofPoint b, ofPoint mid);
    
    ofColor color;
    vector<ofPolyline> lines;
    vector<ofPoint> verts;
    
    bool isComplete;
    int bendIdx; //-1 by default
    
    //Particle goodness:
    //vector<Particle> p;	  //Particles
	//ofFbo fbo;            //Off-screen buffer for trails
    
	float history;        //Control parameter for trails
	float time0;          //Time value for computing dt
    
	float bornRate;       //Particles born rate per second
	float bornCount;      //Integrated number of particles to born
};
