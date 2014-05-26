#include "testApp.h"
#include <time.h>

#define WINDOW_WIDTH 1024

#define APP_STATE_PAUSING 0
#define APP_STATE_ANSWERING 1

#define OPERATOR_IDX_ADDITION 0
#define OPERATOR_IDX_SUBSTRACTION 1
#define OPERATOR_IDX_MULTIPLIER 2

#define MINIMUM_SETTLE_TIME 3

//--------------------------------------------------------------
void testApp::setup(){
    
    
    //using ofxGUI
    ofEnableAntiAliasing();
    min = -1;
    max = 1;
    bufferSize = 256;
    
    
    vector<float> buffer;
    for(int i = 0; i < bufferSize; i++)
    {
        buffer.push_back(0.0);
    }
    
    for (int i = 0; i < NUM_CHANNELS; i++) {
        //initializing channels to 0 ... because c++ doesn't intialize to 0 (non GUI specific)
        values[i] = 0;
        dcOffset[i] = 0;
        channelsEnabled.push_back(true);
    }
    

    red = 0; blue = 0; green = 0;
	ofBackground(red, green, blue, 255);
    
    ofLog(OF_LOG_NOTICE, "Found start");
    
    
    //serial stuff ---- =
    
    serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
	
	// this should be set to whatever com port your serial device is connected to.
	// (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
	// arduino users check in arduino app....
    int baud = OPENBCI_BAUD;
    
    for (int i =0; i<deviceList.size(); ++i) {
        cout << "Trying: " << deviceList[i].getDeviceName() << "\n";
        //bool setupResult= serial.setup("/dev/tty.usbserial-A601LMDE", baud);
        bool setupResult= serial.setup("/dev/" + deviceList[i].getDeviceName(), baud);
        if(setupResult)
        {
            cout << "Success!\n";
            break;
        }
        else
            cout << "Failed. Trying next one... \n";
    }
    
    serial.drain();
    
    //serial.writeByte('b'); For some reason this doesn't work for either character
    //serial.writeByte('f');
    // ---- =
    
    time_t seconds = time(NULL);
    ostringstream filename;
    filename << "/Users/dangoodwin/Desktop/log" << seconds << ".csv";
    cout << "Filename: " << filename.str().c_str();
    logFile.open(filename.str().c_str());
    logFile << "timestamp,prompt,chan0,chan1,chan2,chan3,chan4,chan5,chan6,chan7,\n";
    
    rawDump.open("/Users/dangoodwin/Desktop/rawdump.txt");
    rawDump << "Data:";
    
    //Initialize the appState to be in waiting
    appState = APP_STATE_PAUSING;
    lastStateChangeTime = time(NULL);
    setNewProblem();
    secondsForNextPeriod = 10; //Set a long wait time initially for any settling to occur
    
    verdana30.loadFont("verdana.ttf", 30, true, true);
	verdana30.setLineHeight(34.0f);
	verdana30.setLetterSpacing(1.035);

    lastRecivedData = 0;
}

//--------------------------------------------------------------
void testApp::update(){
    
    //taking in all of the serial data that's available
    std::stringstream ss;
    int availableBytes = serial.available();
    if (availableBytes > 0) {
        if (availableBytes > MAX_BYTES) {
            availableBytes = MAX_BYTES;
        }
        
        serial.readBytes(bytesRead, availableBytes);
        
        //technically this should be able to be factored out... could get simpler
        //right now there's a double buffer.. could happen jsut once
        for (int i = 0; i < availableBytes; i++) {
            rawDump << bytesRead[i]; //For debugging purposes
            byteBuffer.push_back(bytesRead[i]);
        }
    }
    
    //caching buffer
    int totalBytes = byteBuffer.size();
    int lastEnd = 0;
    float currentTime = ofGetElapsedTimef();
    
    //iterate through bytes... looking for packets
    for (int i = 0; i < totalBytes; i++) {
        //packet start found
        if (byteBuffer[i] == PCKT_START) {

            int endIndex = i+1;
            bool foundEnd = false;
            for (; endIndex < totalBytes; endIndex++) {

                // packet end found..
                if (byteBuffer[endIndex] == PCKT_END) {
                    foundEnd = true;
                    lastEnd = endIndex;
                    break;
                }
            }
            
            //once packet end found
            if (foundEnd) {

                
                //cout << "Number of bytes between start and last end " << lastEnd - i << "\n";
                // comments below... help understand structure of data
                // Length of data payload: byteBuffer[i+1]
                // Sample number: byteBuffer[i+2]
                // Beginning of values byteBuffer[i+6]
                
                //Initialize a single line of the logfile
                ostringstream logLine;
                time_t seconds = time(NULL);
                
                logLine << seconds << "," << appState << ",";
                
                for (int channel = 0; channel < NUM_CHANNELS; channel++) {
                    // 4 bytes to integer then normalize to 1
                    // the channel data starts 6 bytes in.. hence i+6
                    // 4 bytes per channel
                    float y = (float)interpretInt32(&byteBuffer[i+6+channel*4]); // /INT_MAX
                    logLine << y << ",";
                    //a few things for noise...
                    //DC offset.. rough filter.. brings the center back to 0... DC offset slowly moves by .1 over time by referencing previous dc offset vs new one
                    //CLAMP - when it pops up it really messes with the DC offset... CLAMP prevents the DC ofset from popping to far
                    //
                    dcOffset[channel] += CLAMP(.1*(y-dcOffset[channel]), -.001, .001);
                    
                    // valuse smoothed using a basic low pass filter
                    // not corrected with DC offset... technically should be values minus dc offset (below)**
                    // .3 smooths raw waveform... but in reality we want this as high as possible...
                    // have a slider can manipulate this in real time
                    //
                    values[channel] += .3*(y-values[channel]);
                    
                    //If we don't care about this channel, don't update the plots
                    if (!channelsEnabled[channel])
                        continue;
                    
                    // happens here **... correcting waveform back to 0
                    float correctedY = values[channel]-dcOffset[channel];
                    //ss << values[channel] << "\n";
                    
                    /* interface sliders to adjust:
                     - dc offset clamp values (-.001, .001) .. magnitude
                     - magnitude amplifier for values adjust
                     - play around with dfts clamping, 1000000, scale by magnitudes of 10
                     -
                     */
                }

                logLine << "\n";
                logFile << logLine.str().c_str();
                lastRecivedData = time(NULL);
                //cout << "Saved line @ " << lastRecivedData << "\n";
                
                //cout << "just tried to write line: " << logLine.str().c_str();
                logLine.clear();
                
                //Because this is where we have completed taking a time sample, write this to the csv
                
            }
            
            i = endIndex;
            
        }
    }
    
    
    if (lastEnd > 0) {
        byteBuffer.erase(byteBuffer.begin(), byteBuffer.begin()+lastEnd);
    }
    
    if (time(NULL)- lastRecivedData>5)
        didReceiveData = false;
    else
        didReceiveData = true;
    
    //------------Figure out which state the app is in------------------//
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
}

void testApp::setNewProblem()
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
    answer = "";
    didAnswer = false;
}
int testApp::interpretInt32(unsigned char *byteArray) {
    
    //interpreting incoming data packets over serial... directly from processing sketch
    //turns bytes into ints... turns 4 bytes into 1 integer
    return (int)(
                 ((0xFF & (int)byteArray[3]) << 24) |
                 ((0xFF & (int)byteArray[2]) << 16) |
                 ((0xFF & (int)byteArray[1]) << 8) |
                 (0xFF & (int)byteArray[0])
                 );
}

//--------------------------------------------------------------
void testApp::draw(){
    
    //Globals, used for both the green bar and line drawing
    totalEnergy = 0.000001;
    alphaSum = 0.0;
    betaSum = 0.0;
    
    //temporary var
    float mag = 0.0;
    
    
    ofSetColor(255, 255, 255);

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
    
        verdana30.drawString(problem_string, WINDOW_WIDTH/2-150, 400);
        verdana30.drawString(answer, WINDOW_WIDTH/2-150, 450);
    }
    else{
        verdana30.drawString("Just relax... :)", WINDOW_WIDTH/2-150, 400);
    }
        
    ostringstream buf;
	buf << "Monitoring channels: ";
    for (int i=0; i<channelsEnabled.size(); ++i) {
        if (channelsEnabled[i])
            buf << i << ",";
    }
    ofDrawBitmapString(buf.str().c_str(), 500, 700);
    

    if(!didReceiveData)
        ofSetColor(255, 0, 0);
    else
        ofSetColor(0, 255, 0);
    
    ofCircle(25, 25, 5);
    
	
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){

    if (key == 'b')
    {
        cout << "STARTED!\n";
        if (!didReceiveData)
            serial.writeByte('b');
    }
    else if (key == 'f')
    {
        cout << "ENGAGED FILTER!\n";
        serial.writeByte('f');
    }
    if (key == 's')
    {
        logFile.close();
    }
    
    else if (key == OF_KEY_RETURN)
    {
        didAnswer = true;
    }
    
    //Disabling/Enabling channels
    else if (key == 'q')
        channelsEnabled[0] = !channelsEnabled[0];
    else if (key == 'w')
        channelsEnabled[1] = !channelsEnabled[1];
    else if (key == 'e')
        channelsEnabled[2] = !channelsEnabled[2];
    else if (key == 'r')
        channelsEnabled[3] = !channelsEnabled[3];
    else if (key == 't')
        channelsEnabled[4] = !channelsEnabled[4];
    else if (key == 'y')
        channelsEnabled[5] = !channelsEnabled[5];
    else if (key == 'u')
        channelsEnabled[6] = !channelsEnabled[6];
    else if (key == 'i')
        channelsEnabled[7] = !channelsEnabled[7];
    
    
    else //Otherwise just consider it part of the answer
    {
        answer += key;
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    oldMouseX = mouseX;
    oldMouseY = mouseY;
    
    mouseY = y;
    mouseX = x;
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
