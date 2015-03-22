///////////////////////////////////////////////////////////////////////////////
//
//This code streams data from the OpenBCI board providing the OpenBCI Hub is
//running. The OpenBCI Hub code can be downloaded from:
//https://github.com/dgoodwin208/OpenBCI_Hub
//This plugin is an OSC-based client that will use the data from
/////////////////////////////////////////////////////////////////////////////

#define PORT 12345

#include "ofxOpenBCIOSC.h"

ofxOpenBCIOSC::ofxOpenBCIOSC()
{

	receiver.setup(PORT);

}

//start the data transfer using the current mode
int ofxOpenBCIOSC::startStreaming() {
    
    return 0;
}

int ofxOpenBCIOSC::stopStreaming() {
    return 0;
}


void ofxOpenBCIOSC::update(bool echoChar) {
    
    unsigned long long timestamp = ofGetElapsedTimeMicros();

    // check for waiting messages
    while(receiver.hasWaitingMessages()){
        
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        
        // check for mouse moved message
        if(m.getAddress() == "/openbci"){
            cout << m.getNumArgs() << "\n";

            
            openBCIPacket dataPacket = openBCIPacket(m.getNumArgs());

            //This needs to be accessible from the data
            dataPacket.sampleIndex = 1; //read the int32 value
            
            //Full doc here: http://docs.openbci.com/software/02-OpenBCI_Streaming_Data_Format#openbci-v3-data-format-binary-format

            for (int i=0; i<m.getNumArgs(); ++i) {
                dataPacket.values[i] = m.getArgAsFloat(i);
            }


            outputPacketBuffer.push(dataPacket);
            

        }        
    }
    
    return;
}

//Takes the latest data out of the output queue and returns it as a vector
vector<openBCIPacket> ofxOpenBCIOSC::getData()
{
    //To be returned in the function
    vector<openBCIPacket> output;
    
    int currentSize =outputPacketBuffer.size();

    for (int i = 0; i<currentSize; ++i) {
        output.push_back(outputPacketBuffer.front());
        outputPacketBuffer.pop();
    }

    return output;
}

//Accessor for to tell client that a new data packet has been parsed from the byte stream.
bool ofxOpenBCIOSC::isNewDataPacketAvailable(){
    return outputPacketBuffer.size()>0;
}


