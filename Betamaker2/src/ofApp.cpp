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
#define PORT 12345

#define DEBUG_MODE 0

#define BUFFER_WEB_LENGTH 10000
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

    plot2 = new ofxHistoryPlot( NULL, "Chan1", 400, false); //NULL cos we don't want it to auto-update. confirmed by "true"
	plot2->setRange(0, ofGetHeight());
	plot2->setColor( ofColor(200,10,200) );
	plot2->setShowNumericalInfo(true);
	plot2->setRespectBorders(true);
	plot2->setLineWidth(3);
    plot2->setAutoRangeShrinksBack(true);

    
    plot1->setDrawGrid(false);
    plot2->setDrawGrid(false);
    
    
    sessionStartTime = time(NULL);
    ostringstream filename;
    
    filename << "/Users/dangoodwin/Desktop/l" << sessionStartTime << ".csv";
    cout << "Filename: " << filename.str().c_str();
    logFile.open(filename.str().c_str());
    logFile << "timestamp,prompt,chan0,chan1,chan2,chan3,chan4,chan5,chan6,chan7,\n";
    
    
    //Set up the math experience
    //Initialize the appState to be in waiting
    appState = APP_STATE_PAUSING;
    lastStateChangeTime = time(NULL);
    setNewProblem();
    
    secondsForNextPeriod = 10; //Set a long wait time initially for any settling to occur
    uploadTimePeriod = 20; //Every 20 seconds we upload the data to the server
    
    verdana30.loadFont("verdana.ttf", 30, true, true);
	verdana30.setLineHeight(34.0f);
	verdana30.setLetterSpacing(1.035);
    
    lastRecivedData = 0;
    
    for (int i=0; i<BUFFER_WEB_LENGTH; ++i) {
        webBuffer.push_back("");
    }
    
    //------------- SET UP TIME SERIES DATA STREAM ------------------//
    if (DEBUG_MODE) {
        int bufferSize = 1024;
        left.assign(bufferSize, 0.0);
        right.assign(bufferSize, 0.0);
        soundStream.listDevices();
        soundStream.setDeviceID(1); //Set the built-in Mac Mic
        soundStream.setup(this, 0, 2, 44100, bufferSize, 4);
    }
    else{
        ofxbci.startStreaming();

    }

    //------------ SET UP DATA TRANSFER TO WEB DATABASE --------------//
    ofAddListener(httpUtils.newResponseEvent,this,&ofApp::newResponse);
	httpUtils.start();
   
    
    
    // open an outgoing connection to HOST:PORT
	//sender.setup(HOST, PORT);

    //Monitor whether a HTTP call is in the works
    uploadingToWeb = false;
    startIdx = 0;
}


void ofApp::reportOSCEvent(){
/*
    ofxOscMessage m;
    m.setAddress("/test");
    m.addIntArg(1);
    m.addFloatArg(3.5f);
    m.addStringArg("hello");
    m.addFloatArg(ofGetElapsedTimef());
    sender.sendMessage(m);
 */
}
//------------------------------------------------------------------------------
void ofApp::update()
{
    
    if(DEBUG_MODE){
        int audio_data_size = left.size();
        for (int i=0; i<audio_data_size; ++i) {
            ostringstream row;
            
            //Update the plots with the latest data from the OpenBCI units
            plot1->update(left[i]);
            plot2->update(right[i]);
            
            //Create the row, which is then pushed both to the logfile and to the server
            row << time(NULL) << ",";
            row << left[i] << ",";
            row << right[i] << ",";
            row << appState << ",";
            row << "\n";
            
            logFile << row; //soon to be removed
            
            //webBuffer.push_back(row.str());
            webBuffer[(startIdx+bufferCtr)%BUFFER_WEB_LENGTH] = row.str();
            bufferCtr+=1;
        }
    }
    
    else{
        //Get any and all bytes off the serial port
        ofxbci.update(false); //Param is to echo to the command line
        if(ofxbci.isNewDataPacketAvailable())
        {
            vector<dataPacket_ADS1299> newData = ofxbci.getData();
            
            int num_packets = newData.size();

            for (int i=0; i<newData.size(); ++i) {
                ostringstream row;
                
                //Update the plots with the latest data from the OpenBCI units
                //newData.printToConsole();
                plot1->update(newData[i].values[0]);
                plot2->update(newData[i].values[1]);
                
                //Create the row, which is then pushed both to the logfile and to the server
                row << newData[i].timestamp - sessionStartTime << ",";
                row << newData[i].values[0] << ",";
                row << newData[i].values[1] << ",";
                row << appState << ",";
                row << "\n";

                logFile << row.str(); //soon to be removed
                
                webBuffer.push_back(row.str());
            }
        }
    
    }
    
    
    
    if (time(NULL) - lastUploadTime> uploadTimePeriod && !uploadingToWeb)
    {
        printf("Trying to upload to the web\n");
        UploadDataToTheWeb();
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
void ofApp::UploadDataToTheWeb(){
    
    ofxHttpForm form;
	form.action = "http://localhost:5000/data";
	form.method = OFX_HTTP_POST;
	
    ostringstream output;
    lastUploadedIdx = webBuffer.size()-1;


    
    for (int i=0; i<=bufferCtr; ++i) {
        output << webBuffer[(startIdx+i)%BUFFER_WEB_LENGTH];
    }
    startIdx = (startIdx + bufferCtr+1)%BUFFER_WEB_LENGTH;
    
    
    form.addFormField("data", output.str() );
    form.addFormField("sessionStartTime", ofToString(sessionStartTime) );
    
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

//---------------Debug tool--------------------------------------
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
