//
//  ofxOpenBCIOSC.h
//  betamaker
//
//  Created by Daniel Goodwin on 3/2/15.
//
//

#ifndef __betamaker__ofxOpenBCIOSC__
#define __betamaker__ofxOpenBCIOSC__

#include "ofMain.h"
#include "ofxOsc.h"
#endif /* defined(__betamaker__ofxOpenBCIOSC__) */

class filterConstants {
public:
    std::vector<double> a, b;
    string name;
    
    filterConstants(const std::vector<double> & b_given, const std::vector<double> & a_given, const string & name_given): b(b_given), a(a_given), name(name_given){
        
    }
};

struct openBCIPacket {
    
public:
    std::vector<float> values;
    int sampleIndex;
    //Timestamp is the universal time (as opposed to relative to an initial timestamp)
    time_t timestamp;
    openBCIPacket(int nValues) : values(nValues, 0){}
    
    int printToConsole() {
        cout <<"printToConsole: dataPacket = ";
        cout << sampleIndex;
        for (int i=0; i < values.size(); i++) {
            cout << ", " << values[i];
        }
        cout << "\n";
        return 0;
    }
    
};





//--------------This is the OpenBCI OpenFrameworks code ------------------//
class ofxOpenBCIOSC {
    
public:
    
    ofxOpenBCIOSC();
    void update(bool echoChar);
    void toggleFilter(bool turnOn);
    void triggerTestSignal(bool turnOn);
    void changeChannelState(int Ichan,bool activate);
    
    int dataMode;
    int startStreaming();
    int stopStreaming();
    bool connectionIsAlive();
    bool filterApplied;
    int streamingMode;
    
    int missedCyclesCounter;
    vector<bool> enabledChannels;
    
    bool isNewDataPacketAvailable();
    vector<openBCIPacket> getData();
    
    
private:
    vector<Byte> currBuffer;
    queue<openBCIPacket> outputPacketBuffer;
    ofxOscReceiver receiver;
};