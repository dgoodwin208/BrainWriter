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

using namespace ofx::IO;


class ofApp: public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);

    void setupNewUser();
    void concludeUserExperience();
    
    //OpenBCI
    ofxOpenBCI ofxbci;
    ofxOpenBCI ofxbci2;
    
    bool hasSentAutoStart;
    bool hasSentApplyFilter;
    ofstream logFile;
    time_t sessionStartTime;
        
    //-------- For posting to the web ---------//
    void newResponse(ofxHttpResponse & response);
    ofxHttpUtils httpUtils;
    vector<string> webBuffer;
    //int webBufferstartIdx; int bufferCtr;//Create a circular buffer
    int uploadTimePeriod;
    time_t lastUploadTime;
    int lastUploadedIdx;
    void UploadDataToTheWeb();
    
    
    //-------- For posting to the OSC -------//
    ofxOscSender sender;
    ofxOscReceiver receiver;
    void reportOSCEvent(int playerNum, float alpha, float beta);
    void reportDebugOSCEvent(string row);
    bool uploadingToWeb;
    
    ofxFft* fft_chan1;
    ofxFft* fft_chan2;
    //vector<vector <float> > timeslicesPerChannel;
    vector<float> timeslice_board1_chan1;
    vector<float> timeslice_board1_chan2;
    vector<float> timeslice_board2_chan1;
    vector<float> timeslice_board2_chan2;

    vector<float> fftoutput_board1_chan1;
    vector<float> fftoutput_board1_chan2;
    vector<float> fftoutput_board2_chan1;
    vector<float> fftoutput_board2_chan2;
};
