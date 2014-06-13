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
#include "ofxHistoryPlot.h"
#include "ofxHttpUtils.h"

using namespace ofx::IO;


class ofApp: public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);

//    std::vector<SerialDevice> devices;
    ofxOpenBCI ofxbci;
    ofxHistoryPlot * plot1; //manual
    ofxHistoryPlot * plot2;
    ofstream logFile;
    
    //-----Mental Math app-----//
    void setNewProblem();
    
    int appState;
    time_t lastStateChangeTime;
    time_t lastRecivedData;
    int secondsForNextPeriod;
    ofTrueTypeFont verdana30;
    bool didAnswer;
    int operand1;
    int operand2;
    int operatorIdx;
    string answer;
    //-------------------------//
    
    //-----Posting to Parse.com//
    void newResponse(ofxHttpResponse & response);
    ofxHttpUtils httpUtils;
};
