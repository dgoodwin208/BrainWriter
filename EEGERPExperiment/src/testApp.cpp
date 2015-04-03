#include "testApp.h"


//How long between sound onset for left
#define LEFT_SOURCE_PERIOD 1500
//How long between sound onset for right
#define RIGHT_SOURCE_PERIOD 1500
//How long after the left sound start do we play the right
#define RIGHT_SOURCE_PERIOD_OFFSET 500.
//How long do we play the sound for
#define TONE_DURATION 300
//Randomness percentage of +- jitter
//Example .1 means source period will randomly range of 90% to 110%
#define SOURCE_DURATION_JITTER .4
//Pause time between experiments (no tone)
#define EXPERIMENT_MODE_CHANGE_BUFFER 1000
#define EXPERIMENT_MODE_CHANGE_PERIOD 6000
#define EXPERIMENT_MODE_REST 0
#define EXPERIMENT_MODE_LEFT 1
#define EXPERIMENT_MODE_RIGHT 2
#define EXPERIMENT_MODE_BUFFER 3

//--------------------------------------------------------------
void testApp::setup(){
    //Load the files (pre-made with an online tool)
	//files: 440.wav, 329.6.wav, burns.wav, woohoo.wav
    leftSource.loadSound("burnscrop.wav"); //
    rightSource.loadSound("woohoo.wav");

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
    

    //Randomness percentage of +- jitter
    //Example .1 means source period will randomly range of 90% to 110%

    //Pause time between experiments (no tone)
    
    logFile << "#Left Source Period: " << LEFT_SOURCE_PERIOD << "\n";
    logFile << "#Right Source Period: " << RIGHT_SOURCE_PERIOD << "\n";
    logFile << "#Right Source Period offset: " << RIGHT_SOURCE_PERIOD_OFFSET << "\n";
    logFile << "#Tone Duration: " << TONE_DURATION << "\n";
    logFile << "#Source Duration Jitter: " << SOURCE_DURATION_JITTER << "\n";
    logFile << "#Experiment Mode Change Buffer: " << EXPERIMENT_MODE_CHANGE_BUFFER << "\n";
    logFile << "#Experiment Mode Change Period: " << EXPERIMENT_MODE_CHANGE_PERIOD << "\n";
    
    
    logFile << "timestamp,expMode,lOnset,rOnset,chan0,chan1,chan2,chan3\n";
    
    if (!logFile.is_open()) {
        cout << "Something wrong with init";
    }
    experimentMode = 0;
    experimentMode_lastChanged = ofGetElapsedTimeMillis();
    
    rSoundCount = 0;
    lSoundCount = 0;
    
}

//--------------------------------------------------------------
void testApp::update(){
    
    unsigned long long timestamp = ofGetElapsedTimeMicros();
	// check for waiting messages
    
    
    ofxbci.update(false); //Param is to echo to the command line
    if(ofxbci.isNewDataPacketAvailable())
    {
        vector<openBCIPacket> newData = ofxbci.getData();
        
        //printf("Sees %lu new packets\n", newData.size());
        
        for (int i=0; i<newData.size(); ++i) {
        
            timestamp = ofGetElapsedTimeMicros();
            
            logFile << timestamp << "," << experimentMode << ",";
            logFile << lOnsetFlag << "," << rOnsetFlag << ",";
            
            //
            if (rOnsetFlag)
                rOnsetFlag = 0;
            if (lOnsetFlag)
                lOnsetFlag = 0;
        
            //Take each of the 8 channels (assuming 8) and stream to file
            for (int j=0; j<8; ++j) {
                logFile << newData[i].values[j] << ",";
            }
            logFile << "\n";
        }
    }

    
    unsigned long long timestamp_mils = timestamp/1000;
    
    //Update the Sound Player
    ofSoundUpdate();

    //------------------Update the tones------------------//
    //Check if the right source (@ ??Hz ) needs to be played
    //We don't play a sound while changing between experiment modes
    if (experimentMode != EXPERIMENT_MODE_BUFFER){
        
        if (ofGetElapsedTimeMillis() - (rightSource_lastPlayed)   >RIGHT_SOURCE_PERIOD){
            rightSource.play();
            rOnsetFlag = 1;
            rSoundCount +=1;
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
            lSoundCount += 1;
            leftSource_lastPlayed = ofGetElapsedTimeMillis();
        }
        else if (ofGetElapsedTimeMillis() - leftSource_lastPlayed >TONE_DURATION)
        {
            leftSource.stop();
        }
    }
    //------------------ Updated the experiment mode ------------------//
    
    if (ofGetElapsedTimeMillis() - experimentMode_lastChanged >EXPERIMENT_MODE_CHANGE_PERIOD + EXPERIMENT_MODE_CHANGE_BUFFER)
    {
        experimentMode = int(ofRandom(2.99));
        experimentMode_lastChanged = ofGetElapsedTimeMillis();
        
        //Reset the time
        leftSource_lastPlayed = ofGetElapsedTimeMillis();
        rightSource_lastPlayed = ofGetElapsedTimeMillis() + 1.*RIGHT_SOURCE_PERIOD_OFFSET*(1. + SOURCE_DURATION_JITTER*ofRandomf());

    }
    //Added in a buffer time between experiments
    else if(ofGetElapsedTimeMillis() - experimentMode_lastChanged >EXPERIMENT_MODE_CHANGE_PERIOD){
        leftSource.stop(); rightSource.stop();
        experimentMode = EXPERIMENT_MODE_BUFFER;

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
    else if (experimentMode== EXPERIMENT_MODE_BUFFER)
        instruction = "";
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
