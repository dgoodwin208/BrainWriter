#include "testApp.h"


//How long between sound onset for left
#define LEFT_SOURCE_PERIOD 1500
//How long between sound onset for right
#define RIGHT_SOURCE_PERIOD 1500
//How long after the left sound start do we play the right
#define RIGHT_SOURCE_PERIOD_OFFSET 500
//How long do we play the sound for
#define TONE_DURATION 300
#define EXPERIMENT_MODE_CHANGE_PERIOD 9000
#define EXPERIMENT_MODE_REST 0
#define EXPERIMENT_MODE_LEFT 1
#define EXPERIMENT_MODE_RIGHT 2


//--------------------------------------------------------------
void testApp::setup(){
    //Load the files (pre-made with an online tool)
	leftSource.loadSound("440.wav");
    rightSource.loadSound("329.6.wav");

    leftSource.setVolume(0.75f);
	rightSource.setVolume(0.75f);

    //Make the left go in the left ear and visa versa
    leftSource.setPan(ofMap(0, 0, 100, -1, 1, true));
    rightSource.setPan(ofMap(100, 0, 100, -1, 1, true));
    // listen on the given port

	current_msg_string = 0;
	mouseX = 0;
	mouseY = 0;
	mouseButtonState = "";

    ofBackground(30, 30, 130);
    
    ofxbci.startStreaming();
    
    rightSource_lastPlayed = ofGetElapsedTimeMillis() + RIGHT_SOURCE_PERIOD_OFFSET;
    leftSource_lastPlayed = ofGetElapsedTimeMillis();

    time_t seconds = time(NULL);
    ostringstream filename;
    filename << "outputfile_timestamp_" << seconds << ".txt";

    cout << "Filename: " << filename.str().c_str() <<"\n";
    logFile.open(filename.str().c_str());
    logFile << "timestamp,expMode,lOnset,rOnset,chan0,chan1,chan2,chan3\n";
    
    if (!logFile.is_open()) {
        cout << "Something wrong with init";
    }
    experimentMode = 0;
    experimentMode_lastChanged = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void testApp::update(){

	// hide old messages
	for(int i = 0; i < NUM_MSG_STRINGS; i++){
		if(timers[i] < ofGetElapsedTimef()){
			msg_strings[i] = "";
		}
	}

    // Hardcoded channel orientation given preset 14 (see evernote doc)
    // Channel ordering is: TP9 FP1 FP2 TP10
    float TP9 = 0.;
    float FP1 = 0.;
    float FP2 = 0.;
    float TP10 = 0.;
    unsigned long long timestamp = ofGetElapsedTimeMicros();
	// check for waiting messages
    
    ofxbci.update(false); //Param is to echo to the command line
    if(ofxbci.isNewDataPacketAvailable())
    {
        vector<openBCIPacket> newData = ofxbci.getData();
        
        //printf("Sees %lu new packets\n", newData.size());
        
        for (int i=0; i<newData.size(); ++i) {
           
            TP9 = newData[i].values[0];
            FP1 = newData[i].values[1];
            FP2 = newData[i].values[2];
            TP10 = newData[i].values[3];
            
            timestamp = ofGetElapsedTimeMicros();
            
            logFile << timestamp << "," << experimentMode << ",";
            logFile << lOnsetFlag << "," << rOnsetFlag << ",";
            
            if (rOnsetFlag)
                rOnsetFlag = 0;
            if (lOnsetFlag)
                lOnsetFlag = 0;
            
            logFile << TP9 << "," << FP1 << "," << FP2 << "," << TP10 << "\n" ;
            //cout << TP9 << "," << FP1 << "," << FP2 << "," << TP10 << "\n" ;
        }
    }

    
    
    //Update the Sound Player
    ofSoundUpdate();

    //------------------Update the tones------------------//
    //Check if the right source (@ ??Hz ) needs to be played
    if (ofGetElapsedTimeMillis() - (rightSource_lastPlayed)   >RIGHT_SOURCE_PERIOD){
        rightSource.play();
        rOnsetFlag = 1;
        rightSource_lastPlayed = ofGetElapsedTimeMillis();
    }
    //Also check if the source needs to be stoppped
    else if (ofGetElapsedTimeMillis() - (rightSource_lastPlayed ) >TONE_DURATION)
    {
        rightSource.stop();
    }
    
    //Check if the left source (@ ?? Hz) needs to be played
    if(ofGetElapsedTimeMillis() - leftSource_lastPlayed  >LEFT_SOURCE_PERIOD){
        leftSource.play();
        lOnsetFlag = 1;
        leftSource_lastPlayed = ofGetElapsedTimeMillis();
    }
    else if (ofGetElapsedTimeMillis() - leftSource_lastPlayed >TONE_DURATION)
    {
        leftSource.stop();
    }
    
    //------------------ Updated the experiment mode ------------------//
    
    if(ofGetElapsedTimeMillis() - experimentMode_lastChanged >EXPERIMENT_MODE_CHANGE_PERIOD){
        experimentMode = int(ofRandom(0.,4.));
        experimentMode_lastChanged = ofGetElapsedTimeMillis();
    }
}


//--------------------------------------------------------------
void testApp::draw(){

	string buf;
	buf = "listening for osc messages on port" + ofToString(PORT);
	ofDrawBitmapString(buf, 10, 20);

	// draw mouse state
	buf = "eeg: " + ofToString(mouseX, 4) +  " " + ofToString(mouseY, 4);
	ofDrawBitmapString(buf, 430, 20);
	ofDrawBitmapString(mouseButtonState, 580, 20);

	for(int i = 0; i < NUM_MSG_STRINGS; i++){
		ofDrawBitmapString(msg_strings[i], 10, 40 + 15 * i);
	}

    //Window is 640 x 480
    //So make a box 200 wide, 100 high
    ofSetColor(255);
    ofRect(220, 190, 200, 100);
    ofSetColor(0);
    string instruction = "REST";
    if (experimentMode== EXPERIMENT_MODE_LEFT)
        instruction = "LEFT";
    else if (experimentMode== EXPERIMENT_MODE_RIGHT)
        instruction = "RIGHT";

    ofDrawBitmapString(instruction, 300, 240);


}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
    if (key == OF_KEY_RETURN)
    {
        cout << "Trying to save file \n";
        logFile.close();
    }
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
