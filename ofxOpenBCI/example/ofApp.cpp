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
    
    ofxbci.startStreaming();
    
    time_t seconds = time(NULL);
    ostringstream filename;
    
    filename << "~/Desktop/l" << seconds << ".csv";
    cout << "Filename: " << filename.str().c_str();
    logFile.open(filename.str().c_str());
    logFile << "timestamp,prompt,chan0,chan1,chan2,chan3,chan4,chan5,chan6,chan7,\n";
    
    
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
            
            logFile << "\n";
        }
    }

    
}


//------------------------------------------------------------------------------
void ofApp::draw()
{
	plot1->draw(10, 10, 1024, 240);
    plot2->draw(10, 300, 1024, 240);
    
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
    
   
}

