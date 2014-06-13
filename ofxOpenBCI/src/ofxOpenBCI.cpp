///////////////////////////////////////////////////////////////////////////////
//
// This class configures and manages the connection to the OpenBCI shield for
// the Arduino.  The connection is implemented via a Serial connection.
// The OpenBCI is configured using single letter text commands sent from the
// PC to the Arduino.  The EEG data streams back from the Arduino to the PC
// continuously (once started).  This class defaults to using binary transfer
// for normal operation.
//
// Created: Chip Audette, Oct 2013
//
// Note: this class does not care whether you are using V1 or V2 of the OpenBCI
// board because the Arduino itself handles the differences between the two.  The
// command format to the Arduino and the data format from the Arduino are the same.
//
//
// Morphed from OpenBCI_ADS1299 to ofxOpenBCI by Daniel Goodwin, May 2014
/////////////////////////////////////////////////////////////////////////////

//#include <string>
#include <algorithm>
#include <time.h>
#import "ofxOpenBCI.h"

#define byte char
#define IS_MAC 1

using namespace ofx::IO;

string command_stop = "s";
string command_startText = "x";
string command_startBinary = "b";
string command_startBinary_4chan = "v";
string command_activateFilters = "F";
string command_deactivateFilters = "f";
string command_deactivated_channel_initializer[] = {"1", "2", "3", "4", "5", "6", "7", "8"};
string command_activated_channel_initializer[] = {"q", "w", "e", "r", "t", "y", "u", "i"};
std::vector<string> command_deactivate_channel(command_deactivated_channel_initializer, command_deactivated_channel_initializer + 8);
std::vector<string> command_activate_channel(command_activated_channel_initializer, command_activated_channel_initializer + 8);

ofxOpenBCI::ofxOpenBCI()
{
    cout << "Trying to set it up...\n";
    dataMode =DATAMODE_BIN;

    int currBuffIndex = 0;
    int num_channels = 8;
    curBuffIndex = 0;
    
    
    //Loop through all available
    //TODO: determine if mac/pc to automatically seed deviceQueryString
    #if IS_MAC
    string deviceQueryString = ".*/tty[.]usb.*";
    #else
    string deviceQueryString = "COM*";
    #endif
    
    std::vector<SerialDeviceInfo> devicesInfo = SerialDeviceUtils::getDevices(deviceQueryString);
//    cout << "Sees " << devicesInfo.size() << " USB devices\n";
    for(std::size_t i = 0; i < devicesInfo.size(); ++i)
    {
        cout << "Trying to connect to: " << devicesInfo[i] <<"\n";
        bool success = serialDevice.setup(devicesInfo[i],
                                          115200,
                                          SerialDevice::DATA_BITS_EIGHT,
                                          SerialDevice::PAR_NONE,
                                          SerialDevice::STOP_ONE,
                                          SerialDevice::FLOW_CTRL_HARDWARE);
        
        if(success)
        {
            ofLogNotice("ofApp::setup") << "Successfully setup " << devicesInfo[i] << "\n";
            break;
        }
        else
        {
            ofLogNotice("ofApp::setup") << "Unable to setup " << devicesInfo[i] << "\n";
        }
    }
    //serial_openBCI(serial);
    
}
//start the data transfer using the current mode
int ofxOpenBCI::startStreaming() {
    
    //stopDataTransfer();
    switch (dataMode) {
        case DATAMODE_BIN:
            serialDevice.writeBytes(command_startBinary + "\n");
            cout << "Processing: OpenBCI_ADS1299: starting binary\n";
            break;
        case DATAMODE_BIN_4CHAN:
            serialDevice.writeBytes(command_startBinary_4chan + "\n");
            cout << "Processing: OpenBCI_ADS1299: starting binary 4-channel\n";
            break;
        case DATAMODE_TXT:
            serialDevice.writeBytes(command_startText + "\n");
            cout << "Processing: OpenBCI_ADS1299: starting text";
            break;
    }
    return 0;
    
}


int ofxOpenBCI::stopStreaming() {
    serialDevice.writeBytes(command_stop + "\n");
    serialDevice.flush(); // clear anything in the com port's buffer
    return 0;
}

//read from the serial port

//simple public interface for a reading singe char of data from BCI device and updating
//the datapacket packet as necessary. (echo writes to console)
void ofxOpenBCI::update(bool echoChar) {
    
    
    uint8_t inByte_array[1];
    
    //If we're having any issue, this is a likely initial culprit
    
    byte inByte;
    int byteCount = 0;
    int bytesAvailable = serialDevice.available();
    uint8_t inByte_arrayBIG[bytesAvailable];

    //Because we see ~160 bytes in the buffer on every update (which is bound to the main app's update fxn)
    currBuffer.clear();
    curBuffIndex=0;
    
    //serialDevice.readByte(inByte_array[0]);
    serialDevice.readBytes(inByte_arrayBIG, bytesAvailable);

//    printf("\n[STARTED] Sees %i Bytes to process \n",bytesAvailable);
    for (int i = 0; i<bytesAvailable; ++i) {
        inByte = inByte_arrayBIG[i];
        
        //byte(inByte_array[0]);
        if (echoChar) //cout << inByte << " ";
            printf("%02X ",inByte);
        
        //accumulate the data in the buffer
        currBuffer.push_back(inByte);
        
        //increment the buffer index for the next time
        curBuffIndex++;
        
        
        //is the data packet complete?
        switch (dataMode) {
            case DATAMODE_BIN:
                if (inByte == BYTE_END){
                    interpretBinaryMessage(curBuffIndex-1);
                }
                break;
            case DATAMODE_BIN_4CHAN:
                if (inByte == BYTE_END) interpretBinaryMessage(curBuffIndex-1);
                break;
            case DATAMODE_TXT:
                printf("[WARNING] TEXT MODE NOT YET SUPPORTED");
                if (inByte == CHAR_END) interpretTextMessage();
                break;
            default:
                //don't accumulate...just reset back to the first place in the buffer
                curBuffIndex=0;
                break;
        }
    }//Finish the for loop around the input bytes

//    printf("\n[DONE]\n");
    //printf("Size of currBuffer: %i bytes \n", currBuffer.size());
    
    return;
}

vector<dataPacket_ADS1299> ofxOpenBCI::getData()
{
    //To be returned in the function
    vector<dataPacket_ADS1299> output;
    
    for (int i = 0; i<outputPacketBuffer.size(); ++i) {
        output.push_back(outputPacketBuffer[i]);
    }
    
    //Clear the internal buffer of completed packets
    outputPacketBuffer.clear();
    return output;
}

//Accessor for to tell client that a new data packet has been parsed from the byte stream.
bool ofxOpenBCI::isNewDataPacketAvailable(){
    return outputPacketBuffer.size()>0;
}



//activate or deactivate an EEG channel...channel counting is zero through nchan-1
void ofxOpenBCI::changeChannelState(int Ichan,bool activate) {
    bool serialPortConnected = true;
    if (serialPortConnected) {
        if ((Ichan >= 0) && (Ichan < command_activate_channel.size())) {
            if (activate) {
                serialDevice.writeBytes(command_activate_channel[Ichan] + "\n");
            } else {
                serialDevice.writeBytes(command_deactivate_channel[Ichan] + "\n");
            }
        }
    }
}

//deactivate an EEG channel...channel counting is zero through nchan-1


//interpret the data. Only called when the last seen byte was BYTE_END
/*
 A Packet looks like this:
 0: Start_char 0xA0
 1: Length of payload
 2-5: Sample number
 6-9: Chan0
 10-13: Chan1
 14: Chan2
 18: Chan3
 22: Chan4
 26: Chan5
 30: Chan6
 34-37: Chan7
 38: End_char 0xC0
 */

int ofxOpenBCI::interpretBinaryMessage(int endInd) {
    
    //assume curBuffIndex has already been incremented to the next open spot
    int startInd = endInd;

    
    //roll backwards to find the start of the packet
    while ((startInd >= 0) && (currBuffer[startInd] != BYTE_START)) {
        startInd--;
    }
    
    if (startInd < 0) {
        // printf("Dropped this packed because it didn't have a start byte\n");
    }
    else if ((endInd - startInd + 1) < 3) {
        // printf("data packet isn't long enough to hold any data...so ignore this data packet\n");
    }
    else {
        int n_bytes = int(currBuffer[startInd + 1]); //this is the number of bytes in the payload
        //printf("Expected %i bytes in this buffer\n", n_bytes);
        
        // check to see if the payload is at least the minimum length
        if (n_bytes < 4*MIN_PAYLOAD_LEN_INT32) {
            //bad data.  ignore this packet;
            //printf("\tAhh it's a runt we have here\n");
        }
        else {
            //check to see if the payload length matches the measured packet size
            if ((startInd + 1 + n_bytes + 1) != endInd) {
                //printf("\tBad packet: %i, %i \n", startInd, endInd);
            }
            else {
                

                int nInt32 = n_bytes / 4;
                
                dataPacket_ADS1299 dataPacket = dataPacket_ADS1299(n_bytes);
                //printf("Bytes 1 and 2: %2X, %2X",currBuffer[startInd], currBuffer[startInd+1]);
                
                dataPacket.sampleIndex = interpretAsInt32(&currBuffer[startInd+2]); //read the int32 value
                //Full doc here: http://www.openbci.com/forums/topic/understanding-serial-interface/
                startInd += 6;  //increment the start index
                
                int nValToRead = min<int>(nInt32-1,dataPacket.values.size());
                for (int i=0; i < nValToRead;i++) {
                    dataPacket.values[i] = interpretAsInt32(&currBuffer[startInd]); //read the int32 value
                    startInd += 4;  //increment the start index
                }
                outputPacketBuffer.push_back(dataPacket);
                //cout << "[Added another Packet!]\n";
            }
        }
    }
 
    return 0;
}

int ofxOpenBCI::interpretAsInt32(byte byteArray[]) {
    //big endian
//    return int(
//               ((0xFF & byteArray[0]) << 24) |
//               ((0xFF & byteArray[1]) << 16) |
//               ((0xFF & byteArray[2]) << 8) |
//               (0xFF & byteArray[3])
//               );
    
    //little endian (worked for Mac)
    return int(
               ((0xFF & byteArray[3]) << 24) |
               ((0xFF & byteArray[2]) << 16) |
               ((0xFF & byteArray[1]) << 8) |
               (0xFF & byteArray[0])
               );
}


int ofxOpenBCI::interpretTextMessage() {
    //still have to code this!
    curBuffIndex=0;  //reset buffer counter back to zero to start refilling the buffer
    return 0;
}



