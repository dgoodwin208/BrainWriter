// =============================================================================
//
// Copyright (c) 2013 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#include "ofApp.h"

//TODO: This should be in code, not a preprocessor directive like this
#define WINDOW_WIDTH 1024

#define APP_STATE_PAUSING 0
#define APP_STATE_ANSWERING 1

#define OPERATOR_IDX_ADDITION 0
#define OPERATOR_IDX_SUBSTRACTION 1
#define OPERATOR_IDX_MULTIPLIER 2

#define MINIMUM_SETTLE_TIME 3

#define HOST "localhost"
#define POST_URL "http://ancient-anchorage-9291.herokuapp.com/data"
#define SENDER_PORT 12345
#define RECEIVER_PORT 6789

#define FREQUENCY_SAMPLING 500
#define ALPHA_START 6
#define ALPHA_END 15
#define BETA_START 15
#define BETA_END 28
#define BUFFER_WEB_LENGTH 10000

#define MAX_OUTPUT_TO_GAME 100
#define DEBUG_MODE 0
//------------------------------------------------------------------------------
void ofApp::setup()
{
    
    //When running a headless application, the framerate can skyrocket unless this is st
    ofSetFrameRate(60);
    
    cout << "In ofApp::setup()\n";
    
    ofxbci.startStreaming();
    ofxbci2.startStreaming();
        
    filtAlpha_player1.setup(4, FREQUENCY_SAMPLING, ALPHA_START, ALPHA_END);
    filtBeta_player2.setup(4, FREQUENCY_SAMPLING, BETA_START, BETA_END);
        
    

    
    //------------ SET UP DATA TRANSFER TO WEB DATABASE --------------//
    ofAddListener(httpUtils.newResponseEvent,this,&ofApp::newResponse);
	httpUtils.start();
    
    //------------ SET UP OSC TO THE GAME  ---------------------------//
    // open an outgoing connection to HOST:PORT
	sender.setup(HOST, SENDER_PORT);
    receiver.setup(RECEIVER_PORT);
    
    //Monitor whether a HTTP call is in the works
    uploadingToWeb = false;
    
    
    //Make the buffer hold one second of data
    int bufferSize = FREQUENCY_SAMPLING;
    fft_chan1 = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING, OF_FFT_FFTW);
    
    fftoutput_board1_chan1.resize(fft_chan1->getBinSize());
    fftoutput_board2_chan1.resize(fft_chan1->getBinSize());
    
    setupNewUser(0);
    setupNewUser(1);
    
    printf("finished setup()\n");
}

//--------------------------------------------------------------
//
void ofApp::setupNewUser(int playerNumber){
    
    
    if (playerNumber==0){
        sessionStartTime_player1 = time(NULL);
        ostringstream filename;
        
        filename << "/Users/dangoodwin/Desktop/l" << sessionStartTime_player1 << "_player1.csv";
        cout << "Filename: " << filename.str().c_str();
        logFile_player1.open(filename.str().c_str());
        
        //Init the user maxes at 1. to avoid any divide by 0 issues
        user1_max_Alpha = 1.;
        user1_max_Beta = 1.;
        user1_last_Alpha =1.;
        user1_last_Beta =1.;
    }
    else{
        sessionStartTime_player2 = time(NULL);
        ostringstream filename;
        
        filename << "/Users/dangoodwin/Desktop/l" << sessionStartTime_player1 << "_player2.csv";
        cout << "Filename: " << filename.str().c_str();
        logFile_player1.open(filename.str().c_str());
        
        user2_max_Alpha = 1.;
        user2_max_Beta = 1.;
        user2_last_Alpha =0.;
        user2_last_Beta =0.;
    }
}


void ofApp::reportDebugOSCEvent(string row){
    ofxOscMessage m;
    m.setAddress("/eegDebug");
    m.addStringArg(row);
    
    sender.sendMessage(m);
    
}


//Transmit normalized values of the alpha and beta per player
//based on the data we've seen so far for that player
void ofApp::reportOSCEvent(int playerNum, float alpha, float beta){
    
    
    ofxOscMessage m;
    
    if (playerNum==1)
        m.setAddress("/player1eeg");
    else
        m.setAddress("/player2eeg");
    
    
    if (playerNum==1){
        
        if (alpha > user1_max_Alpha && alpha <100.){
            user1_max_Alpha = alpha;
        }
        else {
            //We need to return the last value that it transmitted!
            alpha = user1_last_Alpha;
        }
        
        if (beta > user1_max_Beta && beta <100.){
            user1_max_Beta = beta;
        }
        else {
            //We need to return the last value that it transmitted!
            beta = user1_last_Beta;
        }
        
        //Return scaled output
        m.addFloatArg((alpha/user1_max_Alpha)*MAX_OUTPUT_TO_GAME);
        m.addFloatArg((beta/user1_max_Beta)*MAX_OUTPUT_TO_GAME);
    }
    else{
        
        if (alpha > user2_max_Alpha && alpha <100.){
            user2_max_Alpha = alpha;
        }
        else {
            //We need to return the last value that it transmitted!
            alpha = user2_last_Alpha;
        }
        
        if (beta > user2_max_Beta && beta <100.){
            user2_max_Beta = beta;
        }
        else {
            //We need to return the last value that it transmitted!
            beta = user2_last_Beta;
        }
        
        //Return scaled output
        m.addFloatArg((alpha/user2_max_Alpha)*MAX_OUTPUT_TO_GAME);
        m.addFloatArg((beta/user2_max_Beta)*MAX_OUTPUT_TO_GAME);
    }
    
    sender.sendMessage(m);
}

//Demean data inline
vector<float> demeanData(vector<float> data){
    vector<float> output;
    
    float avg = 0.;
    for (int i=0; i<data.size(); ++i) {
        avg+=data[i];
    }
    avg = avg/data.size();
    
    for (int i=0; i<data.size(); ++i) {
        output.push_back(data[i] - avg);
    }
    
    return output;
}

//------------------------------------------------------------------------------
void ofApp::update()
{
    
    /*-----------Making sure that all automatic setup has been complete-----------*/
    //Note that we reference it off the sessionStartTime for player 1, but it's equiv if we key off player2
    if (!hasSentAutoStart && time(NULL)>sessionStartTime_player1+5) {
        ofxbci.startStreaming();
        ofxbci2.startStreaming();
        hasSentAutoStart = true;
    }
    else if (!hasSentApplyFilter && time(NULL)>sessionStartTime_player1+7) {
        ofxbci.toggleFilter(true);
        ofxbci2.toggleFilter(true);
        hasSentApplyFilter = true;
    }
    else if (!hasSentStopOtherChannels && time(NULL)>sessionStartTime_player1+9 ){
        
        printf("Disabling all other channels but 0 and 1\n");
        
        for (int i = 2; i<8; ++i) {
            ofxbci.changeChannelState(i, false);
        }
        //ofxbci.triggerTestSignal(true);
        hasSentStopOtherChannels = true;
        printf("Done");
        
    }
    /*----------------------end auto setup -------------------------------*/
    
    
    /*-------------------- Process Data from the Wire --------------------*/
    //Get any and all bytes off the serial port
    ofxbci.update(false); //Param is to echo to the command line
    ofxbci2.update(false);
    
    vector<dataPacket_ADS1299> newData = ofxbci.getData();

    processNewUserData(1, newData);
    //-----Trying the 2nd openbci board again
    
    
    newData.clear();
    
    newData = ofxbci2.getData();
    processNewUserData(2, newData);
    
    /*-------------------- Process Data from the Wire --------------------*/
    
    
    /*-------------------- Now listen if we reset the game state ---------*/
    
    //Listen if the game has finished with one player
    if(receiver.hasWaitingMessages())
    {
        ofxOscMessage m;
		receiver.getNextMessage(&m);
        
		// check for mouse moved message
		if(m.getAddress() == "/player1score"){
			// both the arguments are int32's
			int player1score = m.getArgAsInt32(0);
            concludeUserExperience(1, player1score);
            setupNewUser(1);
		}
        else if (m.getAddress() == "/player2score"){
            int player2score = m.getArgAsInt32(0);
            concludeUserExperience(2, player2score);
            setupNewUser(2);
        }
        
    }
    
    
    
}


void ofApp::processNewUserData(int playerNum, vector<dataPacket_ADS1299> newData){
    
    
    double filtered_alpha1=0;
    double filtered_beta1=0;
    
    if (playerNum==1){
        for (int i=0; i<newData.size(); ++i) {
            // Note that we're only taking one value off the wire here (there should be at least 2 channels
            filtered_alpha1 = filtAlpha_player1.update(newData[i].values[0]);
            filtered_beta1 = filtBeta_player1.update(newData[i].values[0]);
            
            timeslice_board1_chan1.push_back(newData[i].values[0]);
            
            logFile_player1 << newData[i].values[0] << ",";
            logFile_player1 << newData[i].values[1] << ",";
            logFile_player1 << filtered_alpha1 << ",";
            logFile_player1 << filtered_beta1 << ",";
            
            // Every 1 second of data, calc the FFT and do appropriate steps
            if (timeslice_board1_chan1.size()>1 && timeslice_board1_chan1.size()%FREQUENCY_SAMPLING==0)
            {
                //Supposedly this function should work inline but we need to check
                timeslice_board1_chan1 = demeanData(timeslice_board1_chan1);
                
                fft_chan1->setSignal(timeslice_board1_chan1);
                
                
                float* curFft1 = fft_chan1->getAmplitude();
                
                ostringstream row;
                
                //For now, we will just sum the magnitudes together
                float alpha = 0.;
                float beta=0.;
                
                for (int i= 0; i<fft_chan1->getBinSize(); i++) {
                    fftoutput_board1_chan1[i] = curFft1[i];
                    
                    if (i>ALPHA_START && i<ALPHA_END)
                        alpha+=fftoutput_board1_chan1[i];
                    else if (i>BETA_START && i<BETA_END)
                        beta+=fftoutput_board1_chan1[i];
                    
                    row << fftoutput_board1_chan1[i] << ",";
                }
                printf("Sees %f, %f \n", alpha, beta);
                
                //Do some HEURISTICs to normalize the alpha/beta to good values for the game
                //Empirically, we see good data being created no larger than 100.0, anything above is noise
                
                //Player numbers are 1 and 2
                reportOSCEvent(1, alpha, beta);
                
                timeslice_board1_chan1.clear();
                
            }
        }
    }
    //Now copy the code for player 2
    else{
        for (int i=0; i<newData.size(); ++i) {
            // Note that we're only taking one value off the wire here (there should be at least 2 channels
            filtered_alpha1 = filtAlpha_player2.update(newData[i].values[0]);
            filtered_beta1 = filtBeta_player2.update(newData[i].values[0]);
            
            timeslice_board2_chan1.push_back(newData[i].values[0]);
            
            logFile_player2 << newData[i].values[0] << ",";
            logFile_player2 << newData[i].values[1] << ",";
            logFile_player2 << filtered_alpha1 << ",";
            logFile_player2 << filtered_beta1 << ",";
            
            // Every 1 second of data, calc the FFT and do appropriate steps
            if (timeslice_board2_chan1.size()>1 && timeslice_board2_chan1.size()%FREQUENCY_SAMPLING==0)
            {
                //Supposedly this function should work inline but we need to check
                timeslice_board2_chan1 = demeanData(timeslice_board2_chan1);
                
                fft_chan1->setSignal(timeslice_board2_chan1);
                
                
                float* curFft1 = fft_chan1->getAmplitude();
                
                ostringstream row;
                
                //For now, we will just sum the magnitudes together
                float alpha = 0.;
                float beta=0.;
                
                for (int i= 0; i<fft_chan1->getBinSize(); i++) {
                    fftoutput_board2_chan1[i] = curFft1[i];
                    
                    if (i>ALPHA_START && i<ALPHA_END)
                        alpha+=fftoutput_board2_chan1[i];
                    else if (i>BETA_START && i<BETA_END)
                        beta+=fftoutput_board2_chan1[i];
                    
                    row << fftoutput_board2_chan1[i] << ",";
                }
                printf("Sees %f, %f \n", alpha, beta);
                
                //Do some HEURISTICs to normalize the alpha/beta to good values for the game
                //Empirically, we see good data being created no larger than 100.0, anything above is noise
                
                //Player numbers are 1 and 2
                reportOSCEvent(2, alpha, beta);
                
                timeslice_board2_chan1.clear();
                
            }
        }
        
        
    }
    
    
    return;
}

//------------------------------------------------------------------------------
void ofApp::draw()
{
	
    //Nothing to draw!
    
}

//------------------------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    
    if (key=='b'){
        cout << "YES CAUGHT PRESS";
        ofxbci.startStreaming();
        ofxbci2.startStreaming();
    }
    
    else if (key == 's')
    {
        logFile_player1.close();
    }
    
    else if (key =='f')
    {
        ofxbci.toggleFilter(true);
    }
    else if (key == ' ')
    {
        printf("Disabling all other channels but 0 and 1\n");
        
        for (int i = 2; i<8; ++i) {
            ofxbci.changeChannelState(i, false);
        }
    }
    else if (key == 't')
    {
        ofxbci.triggerTestSignal(true); //haven't implemented the way to turn it off yet ;)
    }
    
    
    
}

void ofApp::audioIn(float * input, int bufferSize, int nChannels){
	
	float curVol = 0.0;
	
	// samples are "interleaved"
	int numCounted = 0;
    
	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume
	for (int i = 0; i < bufferSize; i++){
		left[i]		= input[i*2]*0.5;
		right[i]	= input[i*2+1]*0.5;
        
		curVol += left[i] * left[i];
		curVol += right[i] * right[i];
		numCounted+=2;
	}
}


//--------------------------------------------------------------
//When the user is done with the game, the oF app is responsible for the following:
//Save all the raw data to a .csv file with nomenclature [startSessionTime_score.csv]
//Upload to Twitter (and Pick the best 2-second window to upload)
//Clear out all buffers specific to the user.
void ofApp::concludeUserExperience(int playerNum, int score)
{
    
    printf("UPLOADING CONTENT TO WEB!\n");
    
    float PLOT_SCALING_FACTOR = 10.;
    ofxHttpForm form;
	form.action = POST_URL;
	form.method = OFX_HTTP_POST;
	
    
    //Loop over
    float max = -1000.;
    float min = 1000;
    
    //Get the min and max of the timesample to upload to twitter
    //A very quick heuristic is to use the data in the middle of the player's experience
    int start_idx;
    if (playerNum==1){
        start_idx = (int) ofClamp(rawBuffer_player1.size()/2-FREQUENCY_SAMPLING*2,
                                  rawBuffer_player1.size()-FREQUENCY_SAMPLING*2,
                                  rawBuffer_player1.size());
    }
    else {
        start_idx = (int) ofClamp(rawBuffer_player2.size()/2-FREQUENCY_SAMPLING*2,
                                  rawBuffer_player2.size()-FREQUENCY_SAMPLING*2,
                                  rawBuffer_player2.size());
    }
    //Providing the user played for more than 2 seconds, post it to Twitter :)
    if (start_idx>0) {
        
        for (int i=start_idx; i<start_idx+FREQUENCY_SAMPLING*2; ++i) {
            
            //Each entry is timestamp, leftalpha, leftbeta
            vector<float> row;
            if (playerNum ==1)
                row = rawBuffer_player1[i];
            else
                row = rawBuffer_player2[i];
            
            if (row[1] >max)
                max = row[1];
            if (row[2] >max)
                max = row[2];
            
            if (row[1] < min)
                min = row[1];
            if (row[2] < min)
                min = row[2];
            
        }
        
        float chan1_alpha;
        float chan1_beta;
        float range = max - min;
        
        ostringstream output;
        for (int i=start_idx; i<start_idx+FREQUENCY_SAMPLING*2; ++i) {
            
            //Each entry is timestamp, leftalpha, leftbeta
            vector<float> row;
            if (playerNum ==1)
                row = rawBuffer_player1[i];
            else
                row = rawBuffer_player2[i];
            
            chan1_alpha = ((row[1] - min)/range)*5;
            chan1_beta = ((row[2] - min)/range)*5;
            
            output << chan1_alpha << "," << chan1_beta;
            cout <<chan1_alpha << "," << chan1_beta;
            
            if (i != start_idx+FREQUENCY_SAMPLING*2-1)
                output <<" ";
        }

        if (playerNum==1)
            rawBuffer_player1.clear();
        else
            rawBuffer_player2.clear();
        
        
        form.addFormField("data", output.str() );
        
        if (playerNum==1)
            form.addFormField("username", ofToString(sessionStartTime_player1) );
        else
            form.addFormField("username", ofToString(sessionStartTime_player2) );
        
        ostringstream scoreString;
        scoreString << score;
        form.addFormField("score", scoreString.str());
        
        uploadingToWeb = true;
        httpUtils.addForm(form);
    }
    
    //Finally, close the log files so that can be restarted when we call the SetupUser fxn()
    if (playerNum==1) {
        logFile_player1.flush();
        logFile_player1.close();
    
    }
    else{
    
        logFile_player2.flush();
        logFile_player2.close();

    }
}



void ofApp::newResponse(ofxHttpResponse & response){
    cout << ofToString(response.status) + ": " + (string)response.responseBody;
    lastUploadTime = time(NULL);
    
    //Do anything after we know we successfull posted to Twitter?
    
    uploadingToWeb = false;
}


