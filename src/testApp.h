#pragma once

/*
 This example demonstrates the particles emmiter
 with attraction and spinning forces,
 and also with friction.
 
 It's the example 03-ParticlesForces from the book
 "Mastering openFrameworks: Creative Coding Demystified",
 Chapter 3 - Building a Simple Particle System
 */

#include "ofMain.h"
#include <math.h>
#include "ofxGui.h"
#include "drawnLetter.h"



//openFrameworks' class
class testApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();
    
    //Do time-based processing on the posft vector of mouse positions
    void inferClicks();
    
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
    
    bool hasBeenClosed;
    ofPoint lastClick;
    ofImage image;
    //vector<ofPoint> clicks; //vector of all mouse clicks
    vector<ofPoint> posft; //Mouse positions as function of time
    vector<DrawnLetter> letters; //Letters drawn so far
    
    //Hovering logic
    bool isHoveringOverPoint;
    float hoverTime;
    float startHoverTime;
    float elapsedTime;
    int selectedClicksIdx;
    bool newClick;
};
