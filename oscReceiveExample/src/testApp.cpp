#include "testApp.h"
#include <string> 
#include <sstream>
#include <iostream>
//This code is taken from this SO post
//http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems){
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

//--------------------------------------------------------------
void testApp::setup(){
	// listen on the given port
	cout << "listening for osc messages on port " << PORT << "\n";
	receiver.setup(PORT);
    ofSetFrameRate(40);
    ofSetBackgroundAuto(false);
    ofSetVerticalSync(false);

	current_msg_string = 0;
	mouseX = 0;
	mouseY = 0;
	mouseButtonState = "";

	ofBackground(30, 30, 130);

    for (int i=0; i<126; ++i) {
        fftMags.push_back(0.);
    }
}

//--------------------------------------------------------------
void testApp::update(){

	// hide old messages
	for(int i = 0; i < NUM_MSG_STRINGS; i++){
		if(timers[i] < ofGetElapsedTimef()){
			msg_strings[i] = "";
		}
	}

	// check for waiting messages
	while(receiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage(&m);

		// check for mouse moved message
		if(m.getAddress() == "/eegDebug"){
			// both the arguments are int32's
			string raw = m.getArgAsString(0);
            vector<string> pieces = split(raw, ',');
            
            for (int i=0; i<pieces.size()-1; ++i) {
//                fftMags[i] = (float)std::atof(pieces[i]);
                istringstream(pieces[i])>>fftMags[i];
            }
		}
        if(m.getAddress() == "/player1eeg"){

            alpha = m.getArgAsFloat(0);
            beta = m.getArgAsFloat(1);
            printf("Got vals: %f, %f\n", alpha, beta );
        }
	}
}


//--------------------------------------------------------------
void testApp::draw(){
    
    int width =5;
    
    float max = 0.0;
    for (int i=0; i<fftMags.size(); ++i) {
        if(fftMags[i]>max)
            max=fftMags[i];
    }
    
    //ofPushMatrix();
    //ofRotate(90);

//    for (int i=0; i<fftMags.size(); ++i) {
//        ofRect(width*i, 300-(fftMags[i]/max)*200, width,(fftMags[i]/max)*200 );
//    }

    ofSetColor(25, 25, 25,5);
    ofRect(0, 0,640,480);
    xPos = (xPos+1)%640;

    ofSetColor(0, 0, 255,255);
    ofRect(xPos, 240, 1, alpha);

    ofSetColor(0, 255, 0,255);
    ofPushMatrix();
    ofTranslate(320, 240);
    ofRotate(180);
    ofTranslate(-320, 0);
    ofRect(640-xPos, 0, 1, beta);
    ofPopMatrix();

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

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
