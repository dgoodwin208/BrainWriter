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
//#include "processing_math.h"

//Particle class
class Particle {
public:
	Particle();                //Class constructor
	void setup();              //Start particle
	void update( float dt );   //Recalculate physics
	void draw();               //Draw particle
    
	ofPoint pos;               //Position
	ofPoint vel;               //Velocity
	float time;                //Time of living
	float lifeTime;            //Allowed lifetime
	bool live;                 //Is particle live
};

//Control parameters class
class Params {
public:
	void setup();
	ofPoint eCenter;    //Emitter center
	float eRad;         //Emitter radius
	float velRad;       //Initial velocity limit
	float lifeTime;     //Lifetime in seconds
    
	float rotate;   //Direction rotation speed in angles per second
    
	float force;       //Attraction/repulsion force inside emitter
	float spinning;    //Spinning force inside emitter
	float friction;    //Friction, in the range [0, 1]
};

//Control parameters class
class DrawnLetter {
public:
    DrawnLetter();
    void setup();
    void deleteLast();
    void draw();
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
    vector<Particle> p;	  //Particles
	ofFbo fbo;            //Off-screen buffer for trails
    
	float history;        //Control parameter for trails
	float time0;          //Time value for computing dt
    
	float bornRate;       //Particles born rate per second
	float bornCount;      //Integrated number of particles to born
    
    
    
};

extern Params param; //Declaration a of global variable


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
