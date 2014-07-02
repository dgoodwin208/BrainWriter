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


#pragma once


#include "ofMain.h"
#include "ofxOpenBCI.h"
#include "ofxHttpUtils.h"
#include "ofxOsc.h"
#include "ofxFft.h"
#include "ofxInlineFilter.h"




class ofApp: public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);

    void setupNewUser(int playerNumber);
    void concludeUserExperience(int playerNum, int score);
    
    void processNewUserData(int playerNum, vector<dataPacket_ADS1299> input);
    //------------------OpenBCI----------------//
    ofxOpenBCI ofxbci;
    ofxOpenBCI ofxbci2;
    
    //------------Filters for alpha beta ------//
    ofxInlineFilter filtAlpha_player1;
    ofxInlineFilter filtBeta_player1;
    ofxInlineFilter filtAlpha_player2;
    ofxInlineFilter filtBeta_player2;
    
    
    //-------------Auto start bools -----------//
    bool hasSentAutoStart;
    bool hasSentApplyFilter;
    bool hasSentStopOtherChannels;
    
    //---------Debugging tools, using mic input -----//
    ofSoundStream soundStream;
    void audioIn(float * input, int bufferSize, int nChannels);
    vector <float> left;
    vector <float> right;
    bool hasSentSampleUpload;
    
    
        
    //-------- For posting to the web ---------//
    void newResponse(ofxHttpResponse & response);
    ofxHttpUtils httpUtils;

    
    //User Specific Variables for a particular play session :)
    ofstream logFile_player1;
    ofstream logFile_player2;
    std::vector<vector<float> > rawBuffer_player1;
    std::vector<vector<float> > rawBuffer_player2;
    
    time_t sessionStartTime_player1;
    time_t sessionStartTime_player2;
    
    float user1_max_Alpha;
    float user1_max_Beta;
    float user2_max_Alpha;
    float user2_max_Beta;
    
    float user1_last_Alpha;
    float user1_last_Beta;
    float user2_last_Alpha;
    float user2_last_Beta;
    
    
    

    int uploadTimePeriod;
    time_t lastUploadTime;
    void UploadDataToTheWeb();
    
    
    //-------- For posting to the OSC -------//
    ofxOscSender sender;
    ofxOscReceiver receiver;
    void reportOSCEvent(int playerNum, float alpha, float beta);
    void reportDebugOSCEvent(string row);
    bool uploadingToWeb;
    
    ofxFft* fft_chan1;

    
    //---------The battery of vectors to store time domain data-----//
    vector<float> timeslice_board1_chan1;
    vector<float> timeslice_board2_chan1;

    vector<float> fftoutput_board1_chan1;
    vector<float> fftoutput_board2_chan1;
};
