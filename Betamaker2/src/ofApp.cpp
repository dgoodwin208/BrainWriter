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


//------------------------------------------------------------------------------
void ofApp::setup()
{

    cout << "In ofApp::setup()\n";

    //The numerical parameter is the length of the history
    plot1 = new ofxHistoryPlot( NULL, "Chan0", 400, false); //NULL cos we don't want it to auto-update. confirmed by "true"
	plot1->setRange(0, ofGetHeight());
	plot1->setColor( ofColor(200,10,200) );
	plot1->setShowNumericalInfo(true);
	plot1->setRespectBorders(true);
	plot1->setLineWidth(3);
    plot1->setAutoRangeShrinksBack(true);

    plot2 = new ofxHistoryPlot( NULL, "Chan0", 400, false); //NULL cos we don't want it to auto-update. confirmed by "true"
	plot2->setRange(0, ofGetHeight());
	plot2->setColor( ofColor(200,10,200) );
	plot2->setShowNumericalInfo(true);
	plot2->setRespectBorders(true);
	plot2->setLineWidth(3);
    plot2->setAutoRangeShrinksBack(true);

    
    plot1->setDrawGrid(false);
    plot2->setDrawGrid(false);
    
    ofxbci.startStreaming();
    
    time_t seconds = time(NULL);
    ostringstream filename;
    
    filename << "/Users/dangoodwin/Desktop/l" << seconds << ".csv";
    cout << "Filename: " << filename.str().c_str();
    logFile.open(filename.str().c_str());
    logFile << "timestamp,prompt,chan0,chan1,chan2,chan3,chan4,chan5,chan6,chan7,\n";
    
    
    //Set up the math experience
    //Initialize the appState to be in waiting
    appState = APP_STATE_PAUSING;
    lastStateChangeTime = time(NULL);
    setNewProblem();
    secondsForNextPeriod = 10; //Set a long wait time initially for any settling to occur
    
    verdana30.loadFont("verdana.ttf", 30, true, true);
	verdana30.setLineHeight(34.0f);
	verdana30.setLetterSpacing(1.035);
    
    lastRecivedData = 0;
    
    
    ofAddListener(httpUtils.newResponseEvent,this,&ofApp::newResponse);
	httpUtils.start();
    
    ofxHttpForm form;
	form.action = "http://localhost:5000/data";
	form.method = OFX_HTTP_POST;
	form.addFormField("data", ofToString("delivered"));
    
	httpUtils.addForm(form);

	
    //requestStr = "message sent: " + ofToString(counter);
	
    
    
    
    
}

//------------------------------------------------------------------------------
void ofApp::update()
{
    
    //Get any and all bytes off the serial port
    ofxbci.update(false); //Param is to echo to the command line
    if(ofxbci.isNewDataPacketAvailable())
    {
        vector<dataPacket_ADS1299> newData = ofxbci.getData();
        
        printf("Sees %i new packets\n", newData.size());

        for (int i=0; i<newData.size(); ++i) {
            plot1->update(newData[i].values[0]);
            logFile << newData[i].values[0] << ",";
            
            plot2->update(newData[i].values[1]);
            logFile << newData[i].values[1] << ",";
            
            logFile << appState << ",";
            logFile << "\n";
        }
    }

    //------------Figure out which state the drawing app is in------------------//
    time_t timeElapsedSeconds = time(NULL) - lastStateChangeTime;
    if (appState == APP_STATE_PAUSING)
    {
        //Is the app beyond the wait time?
        if (timeElapsedSeconds>secondsForNextPeriod){
            appState = APP_STATE_ANSWERING;
            lastStateChangeTime = time(NULL);
            secondsForNextPeriod = -1;
            setNewProblem();
        }
    }
    
    else if (appState == APP_STATE_ANSWERING)
    {
        //If the person answers the question before MINIMUM_SETTLE_TIME, give them a new problem
        if (didAnswer && timeElapsedSeconds<MINIMUM_SETTLE_TIME){
            setNewProblem();
        }
        //If the person has answered the question, then set the app back
        else if (didAnswer && timeElapsedSeconds >= MINIMUM_SETTLE_TIME)
        {
            appState = APP_STATE_PAUSING;
            lastStateChangeTime = time(NULL) ;
            secondsForNextPeriod = rand() %10 + MINIMUM_SETTLE_TIME;
        }
    }
    //----------end draw app part of the update function ---------//
    
}

void ofApp::setNewProblem()
{
    operatorIdx= rand() % 3;
    operand1 = rand() % 100;
    operand2 = rand() % 100;
    
    //If the operator will be mult, then we shrink the operands
    if (operatorIdx == OPERATOR_IDX_MULTIPLIER)
    {
        operand1 = rand() % 30;
        operand2 = rand() % 10;
    }
    
    //Reset the answer
    //answer = "";
    didAnswer = false;
}

//------------------------------------------------------------------------------
void ofApp::draw()
{
	plot1->draw(10, 10, 1024, 240);
    plot2->draw(10, 300, 1024, 240);
    //ofBackground(0);

    if (appState == APP_STATE_ANSWERING){
        string problem_string;
        problem_string += ofToString(operand1);
        if (operatorIdx == OPERATOR_IDX_ADDITION)
            problem_string += '+';
        else if ( operatorIdx == OPERATOR_IDX_SUBSTRACTION)
            problem_string += '-';
        else
            problem_string += '*';
        
        problem_string += ofToString(operand2);
        problem_string += '=';
        
        verdana30.drawString(problem_string, WINDOW_WIDTH/2-150, 600);
        verdana30.drawString(answer, WINDOW_WIDTH/2-150, 650);
    }
    else{
        verdana30.drawString("Just relax... :)", WINDOW_WIDTH/2-150, 600);
    }


}

//------------------------------------------------------------------------------
void ofApp::keyPressed(int key)
{

    if (key=='b'){
        ofxbci.startStreaming();
    }
    
    else if (key == 's')
    {
        logFile.close();
    }
    
    //---------This is part of the arithmetic app---------//
    else if (key == OF_KEY_RETURN)
    {
        didAnswer = true;
    }
    else //Otherwise just consider it part of the arithmetic answer
    {
        answer += key;
    }
    //---------This is part of the arithmetic app---------//
    
}

//--------------------------------------------------------------
void ofApp::newResponse(ofxHttpResponse & response){
    cout << ofToString(response.status) + ": " + (string)response.responseBody;
}

