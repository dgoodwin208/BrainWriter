#pragma once

#include <math.h>
#include "ofMain.h"


class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void setNewProblem();
    
    int interpretInt32(unsigned char *byteArray);
    
    float red, green, blue;
    int min, max;
    int bufferSize;
    
    
    //Plot line
    vector<ofVec3f> points;
    vector<ofFloatColor> colors;
    vector<double> weights;
    
    int mouseX, mouseY;
    int oldMouseY, oldMouseX;
    float totalEnergy;
    float alphaSum;
    float betaSum;
    
    ofSoundPlayer 		alphaReward;
    ofSoundPlayer       betaReward;
    
    float manual_energy_alpha;
    float manual_energy_beta;
    
    vector <bool> channelsEnabled;
    ofstream logFile;
    ofstream rawDump;

    
    bool userIsThinking;
    int appState;
    time_t lastStateChangeTime;
    time_t lastRecivedData;
    bool didReceiveData;
    int secondsForNextPeriod;
    
    int operand1;
    int operand2;
    int operatorIdx;
    bool didAnswer;

    string answer;
    
    //make a nice string of it
    ofTrueTypeFont verdana30;
};

