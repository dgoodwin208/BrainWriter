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

#define ALPHA_START 6
#define ALPHA_END 15
#define BETA_START 15
#define BETA_END 28
#define BUFFER_WEB_LENGTH 10000

//------------------------------------------------------------------------------
void ofApp::setup()
{
    
    //When running a headless application, the framerate can skyrocket unless this is st
    ofSetFrameRate(60);
    
    cout << "In ofApp::setup()\n";
    
    
    ofxbci.startStreaming();
    ofxbci2.startStreaming();
    
    
    //------------ SET UP DATA TRANSFER TO WEB DATABASE --------------//
    ofAddListener(httpUtils.newResponseEvent,this,&ofApp::newResponse);
	httpUtils.start();
    
    //------------ SET UP OSC TO THE GAME  ---------------------------//
    // open an outgoing connection to HOST:PORT
	sender.setup(HOST, SENDER_PORT);
    receiver.setup(RECEIVER_PORT);
    
    //Monitor whether a HTTP call is in the works
    uploadingToWeb = false;
    //webBufferstartIdx = 0;
    
    
    int bufferSize = 256;
    fft_chan1 = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING, OF_FFT_FFTW);
    fft_chan2 = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING, OF_FFT_FFTW);

    fftoutput_board1_chan1.resize(fft_chan1->getBinSize());
    fftoutput_board1_chan2.resize(fft_chan2->getBinSize());
    fftoutput_board2_chan1.resize(fft_chan1->getBinSize());
    fftoutput_board2_chan2.resize(fft_chan2->getBinSize());
    
    setupNewUser();
    
    printf("finished setup()\n");
}

//--------------------------------------------------------------
//
void ofApp::setupNewUser(){
    
    sessionStartTime = time(NULL);
    ostringstream filename;
    
    filename << "/Users/dangoodwin/Desktop/l" << sessionStartTime << ".csv";
    cout << "Filename: " << filename.str().c_str();
    logFile.open(filename.str().c_str());
    //logFile << "timestamp,prompt,chan0,chan1,chan2,chan3,chan4,chan5,chan6,chan7,\n";
}


void ofApp::reportDebugOSCEvent(string row){
    ofxOscMessage m;
    m.setAddress("/eegDebug");
    m.addStringArg(row);
    
    sender.sendMessage(m);
    
}

void ofApp::reportOSCEvent(int playerNum, float alpha, float beta){
    ofxOscMessage m;
    
    if (playerNum==1)
        m.setAddress("/player1eeg");
    else
        m.setAddress("/player2eeg");
    
    m.addFloatArg(alpha);
    m.addFloatArg(beta);
    
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
    //Data filtering will be done this way:
    //http://stackoverflow.com/questions/664877/i-need-to-implement-a-butterworth-filter-in-c-is-it-easier-get-a-library-with-t
    if (!hasSentAutoStart && time(NULL)>sessionStartTime+5) {
        ofxbci.startStreaming();
        ofxbci2.startStreaming();
        hasSentAutoStart = true;
    }
    else if (!hasSentApplyFilter && time(NULL)>sessionStartTime+7) {
        ofxbci.toggleFilter(true);
        ofxbci2.toggleFilter(true);
        hasSentApplyFilter = true;
    }
    else if (!hasSentStopOtherChannels && time(NULL)>sessionStartTime+9 ){
        
        printf("Disabling all other channels but 0 and 1\n");
        
        for (int i = 2; i<8; ++i) {
            ofxbci.changeChannelState(i, false);
        }
        hasSentStopOtherChannels = true;

    }
        
    //Get any and all bytes off the serial port
    ofxbci.update(false); //Param is to echo to the command line
    
    vector<dataPacket_ADS1299> newData = ofxbci.getData();
    
    int num_packets = newData.size();
    //printf("Seeing %i packets on the first interface\n", num_packets);
    
    for (int i=0; i<newData.size(); ++i) {
        // Note that we're only taking one value off the wire here (there should be at least 2 channels
        timeslice_board1_chan1.push_back(newData[i].values[0]);
        timeslice_board1_chan2.push_back(newData[i].values[1]);
        //printf("Incoming:  %f, %f \n", newData[i].values[0], newData[i].values[1]);
        // Every 1 second of data (250 entries), calc the FFT and do appropriate steps
        if (timeslice_board1_chan1.size()>1 && timeslice_board1_chan1.size()%250==0)
        {
            
            //Demean the timeslice first:
            //Supposedly this function should work inline but we need to check
            timeslice_board1_chan1 = demeanData(timeslice_board1_chan1);
            timeslice_board1_chan2 = demeanData(timeslice_board1_chan2);

            fft_chan1->setSignal(timeslice_board1_chan1);
            fft_chan2->setSignal(timeslice_board1_chan2);

            float* curFft1 = fft_chan1->getAmplitude();
            float* curFft2 = fft_chan2->getAmplitude();
            ostringstream row;

            
            //For now, we will just sum the magnitudes together
            float alpha = 0.;
            float beta=0.;
            float max= 0.;
            for (int i= 0; i<fft_chan1->getBinSize(); i++) {
                fftoutput_board1_chan1[i] = curFft1[i];
                fftoutput_board1_chan2[i] = curFft2[i];
                
                if (i>ALPHA_START && i<ALPHA_END)
                    alpha+=fftoutput_board1_chan1[i]+fftoutput_board1_chan2[i];
                else if (i>BETA_START && i<BETA_END)
                    beta+=fftoutput_board1_chan1[i]+fftoutput_board1_chan2[i];
                printf("Sees %f, %f \n", alpha, beta);
                row << fftoutput_board1_chan1[i]+fftoutput_board1_chan2[i] << ",";
            }
            
            //reportDebugOSCEvent(row.str());
            reportOSCEvent(1, alpha, beta);
            
            
            timeslice_board1_chan1.clear();
            timeslice_board1_chan2.clear();

        }
    }
    
    
    
    //-----Trying the 2nd openbci board again
    
    ofxbci2.update(false); //Param is to echo to the command line
    newData.clear();
    newData = ofxbci2.getData();
    
    num_packets = newData.size();
    //printf("Seeing %i packets on the first interface\n", num_packets);
    
    for (int i=0; i<newData.size(); ++i) {
        // Note that we're only taking one value off the wire here (there should be at least 2 channels
        timeslice_board2_chan1.push_back(newData[i].values[0]);
        timeslice_board2_chan2.push_back(newData[i].values[1]);
        // Every 1 second of data (250 entries), calc the FFT and do appropriate steps
        if (timeslice_board2_chan1.size()>1 && timeslice_board2_chan1.size()%250==0)
        {
            
            //Demean the timeslice first:
            
            fft_chan1->setSignal(timeslice_board2_chan1);
            fft_chan2->setSignal(timeslice_board2_chan2);
            float* curFft1 = fft_chan1->getAmplitude();
            float* curFft2 = fft_chan2->getAmplitude();
            
            ostringstream row;
            for (int i= 0; i<fft_chan1->getBinSize(); i++) {
                fftoutput_board2_chan1[i] = curFft1[i];
                fftoutput_board2_chan2[i] = curFft2[i];
                row << fftoutput_board2_chan1[i] + fftoutput_board2_chan2[i]  << ",";
            }
            
           // reportDebugOSCEvent(row.str());
            
            timeslice_board2_chan1.clear();
            timeslice_board2_chan2.clear();
            
            
            //logFile << row.str(); //soon to be removed
            //bbwebBuffer.push_back(row.str());
        }
    }

    //--------End trying with the second openbci board
    
    
    
    
    //Listen if the game has finished with one player
    if(receiver.hasWaitingMessages())
    {
        ofxOscMessage m;
		receiver.getNextMessage(&m);
        
		// check for mouse moved message
		if(m.getAddress() == "/player1score"){
			// both the arguments are int32's
			int player1score = m.getArgAsInt32(0);
		}
        
    }
    
    
    //    if (time(NULL) - lastUploadTime> uploadTimePeriod && !uploadingToWeb)
    //    {
    //        printf("Trying to upload to the web\n");
    //        UploadDataToTheWeb();
    //    }
    
    
    
}


//------------------------------------------------------------------------------
void ofApp::draw()
{
	
    
//    ofPushStyle();
//    ofPushMatrix();
//    ofTranslate(32, 170, 0);
//    
//    ofSetColor(225);
//    ofDrawBitmapString("Left Channel", 4, 18);
//    
//    ofSetLineWidth(1);
//    ofRect(0, 0, 512, 200);
//    
//    ofSetColor(245, 58, 135);
//    ofSetLineWidth(2);
//    
//    ofBeginShape();
//    for (unsigned int i = 0; i < fftoutput.size(); i++){
//        ofVertex(i*2, fftoutput[i]);
//    }
//    ofEndShape(false);
//    
//    ofPopMatrix();
//	ofPopStyle();
//    
    
}

//------------------------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    
    if (key=='b'){
        cout << "YES CAUGHT PRESS";
        ofxbci.startStreaming();
        //ofxbci2.startStreaming();
    }
    
    else if (key == 's')
    {
        logFile.close();
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


//--------------------------------------------------------------
//When the user is done with the game, the oF app is responsible for the following:
//Save all the raw data to a .csv file with nomenclature [startSessionTime_score.csv]
//Upload to Twitter (and Pick the best 2-second window to upload)
//Clear out all buffers specific to the user.
void ofApp::concludeUserExperience()
{
    ofxHttpForm form;
	form.action = POST_URL;
	form.method = OFX_HTTP_POST;
	
    ostringstream output;
    
    lastUploadedIdx = webBuffer.size()-1;
    
    
    //Loop through the user's time history to find the best amplitude
    //cout << "Doooone!\n";
//    fft_chan1->setSignal(timeslice);
//    fft_chan2->setSignal(timeslice);
//    
//    float* curFft = fft->getAmplitude();
    
    //    //memcpy(&fftoutput[0], curFft, sizeof(float) * fft->getBinSize());
    //    for (int i= 0; i<fft->getBinSize(); i++) {
    //        row << curFft[i] << ",";
    //    }
    //    row << "\n";
    
    
    
    //Choose the index that is most likely going to give us 500 time samples
    int mid_index = min(lastUploadedIdx/2,lastUploadedIdx-500);
    //If we still don't have a valid index, get out
    if (lastUploadedIdx<0)
        printf("ERROR: trying to upload to the web without 2 seconds of data");
    webBuffer.clear();
    return;
    
    //Otherwise, make a csv file that can be uploaded to the web
    for (int i=mid_index; i<mid_index+500; ++i) {
        output << webBuffer[i%BUFFER_WEB_LENGTH];
    }
    
    //Reset the
    //startIdx = (webBufferstartIdx + bufferCtr+1)%BUFFER_WEB_LENGTH;
    
    form.addFormField("data", output.str() );
    form.addFormField("username", ofToString(sessionStartTime) );
    form.addFormField("score","90210");
    uploadingToWeb = true;
	httpUtils.addForm(form);
}



void ofApp::newResponse(ofxHttpResponse & response){
    cout << ofToString(response.status) + ": " + (string)response.responseBody;
    lastUploadTime = time(NULL);
    
    //Now that we know it uploaded correctly, remove the data from the webBuffer
    /*
     for (int i=0; i<lastUploadedIdx; ++i) {
     webBuffer.erase(webBuffer.begin());
     }
     */
    
    uploadingToWeb = false;
}
