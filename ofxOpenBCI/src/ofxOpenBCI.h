//
//  ofxOpenBCI.h
//  
//
//  Created by Daniel Goodwin on 4/08/13.
//
//

#include <string.h>
#include "ofMain.h"
#include "ofxSerial.h"

using namespace ofx::IO;

#define OPENBCI_BAUDRATE 115200
#define byte char

const int DATAMODE_TXT = 0;
const int DATAMODE_BIN = 1;
const int DATAMODE_BIN_4CHAN = 4;

const int STATE_NOCOM = 0;
const int STATE_COMINIT = 1;
const int STATE_NORMAL = 2;
const int COM_INIT_MSEC = 4000; //you may need to vary this for your computer or your Arduino

const byte BYTE_START = byte(0xA0);
const byte BYTE_END = byte(0xC0);
const byte CHAR_END = byte(0xA0);  //line feed?
const int LEN_SERIAL_BUFF_CHAR = 1000;
const int MIN_PAYLOAD_LEN_INT32 = 1; //8 is the normal number, but there are shorter modes to enable Bluetooth


struct dataPacket_ADS1299 {

    public:
        std::vector<int> values;
        int sampleIndex;
        //Timestamp is the universal time (as opposed to relative to an initial timestamp)
        time_t timestamp;
    dataPacket_ADS1299(int nValues) : values(nValues, 0){}
    
//    dataPacket_ADS1299(int nValues){
//        for (int i=0; i < nValues; i++) {
//            values.push_back(0);
//        }
//    }
    int printToConsole() {
        cout <<"printToConsole: dataPacket = ";
        cout << sampleIndex;
        for (int i=0; i < values.size(); i++) {
            cout << ", " << values[i];
        }
        cout << "\n";
        return 0;
    }
    int copyTo(dataPacket_ADS1299 & target) {
        target.sampleIndex = sampleIndex;
        for (int i=0; i < values.size(); i++) {
            target.values[i] = values[i];
        }
        return 0;
    }
};


class filterConstants {
public:
    std::vector<double> a, b;
    string name;
    
    filterConstants(const std::vector<double> & b_given, const std::vector<double> & a_given, const string & name_given): b(b_given), a(a_given), name(name_given){
        
    }
};



//--------------This is the OpenBCI OpenFrameworks code ------------------//
class ofxOpenBCI {
public:
        

    ofxOpenBCI();
    void update(bool echoChar);
    void toggleFilter(bool turnOn);
    void triggerTestSignal(bool turnOn);
    void changeChannelState(int Ichan,bool activate);
    
    vector<dataPacket_ADS1299> getData();

    SerialDevice serialDevice;

    int dataMode;
    int startStreaming();
    int stopStreaming();
    bool connectionIsAlive();
    bool filterApplied;
    int streamingMode;
    
    vector<bool> enabledChannels;

    bool isNewDataPacketAvailable();
    private: int interpretBinaryMessage();
    int interpretBinaryMessageForward (int endInd);
    
    int interpretBinaryMessage(int endInd);
    private: int interpretTextMessage();
    
    private: int curBuffIndex;
    private: int interpretAsInt32(byte byteArray[]);
    vector<byte>leftoverBytes;

    private: vector<byte> currBuffer;
    private: queue<dataPacket_ADS1299> outputPacketBuffer;
    

};

