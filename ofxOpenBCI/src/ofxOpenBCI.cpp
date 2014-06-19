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
// Serial C++ code adapted from Jon Weisz's code here
//          available here: https://github.com/jon-weisz/openbci/tree/master/libopenbci
// Morphed from OpenBCI_ADS1299 to ofxOpenBCI by Daniel Goodwin, May 2014
/////////////////////////////////////////////////////////////////////////////

//#include <string>
#include <algorithm>
#include <time.h>
#include <sys/time.h>

#import "ofxOpenBCI.h"

#define byte char
#define IS_MAC 1
#define MAX_LEFTOVER_SERIAL_BYTES 200
using namespace ofx::IO;

string command_stop = "s";
string command_startText = "x";
string command_startBinary = "b";
string command_startBinary_4chan = "v";
string command_activateFilters = "F";
string command_deactivateFilters = "f";
string command_start_test_signal = "+";
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

void ofxOpenBCI::toggleFilter(bool turnOn)
{
    if (turnOn){
        serialDevice.writeBytes(command_activateFilters + "\n");
        cout << "Processing: OpenBCI_ADS1299: engaging filter\n";
    }
    else
    {
        serialDevice.writeBytes(command_deactivateFilters + "\n");
        cout << "Processing: OpenBCI_ADS1299: deactivating filter\n";
    }
        
}
void ofxOpenBCI::triggerTestSignal(bool turnOn)
{
    serialDevice.writeBytes(command_start_test_signal + "\n");
    cout << "Generating test signal\n";
}

int ofxOpenBCI::stopStreaming() {
    serialDevice.writeBytes(command_stop + "\n");
    serialDevice.flush(); // clear anything in the com port's buffer
    return 0;
}

//read from the serial port

//simple public interface for a reading singe char of data from BCI device and updating
//the datapacket packet as necessary. (echo writes to console)
//AT a sample rate of 250Hz we see about ~150 bytes per call of the update function
void ofxOpenBCI::update(bool echoChar) {
    
    struct timeval tv;
    gettimeofday(&tv, 0);

    
    //First, see if there's anything in the leftover buffer and push into the currBuffer
    curBuffIndex=0;
    currBuffer.clear();
    for (int i=0; i<leftoverBytes.size(); ++i) {
        currBuffer.push_back(leftoverBytes[i]);
        curBuffIndex++;
    }


    
    //Then, get all bytes off the serial port
    int bytesAvailable = serialDevice.available();
    printf("UPDATE START (%i):\n",bytesAvailable);//, tv.tv_usec
    uint8_t inByte_arrayBIG[bytesAvailable];
    serialDevice.readBytes(inByte_arrayBIG, bytesAvailable);

    byte inByte;
    for (int i = 0; i<bytesAvailable; ++i) {
        inByte = inByte_arrayBIG[i];
        
        //This is very weird, but sometimes the last element of the input buffer from the serial port
        //is repeated as the first character in the new input buffer
        
        /*
        if (i==0 && leftoverBytes.size()>0){
            printf("%02X =? %02X\n",inByte,leftoverBytes.back());
            if (inByte==leftoverBytes.back()){
                printf("SKIPPING ERRONEOUS DUPLICATE\n");
                continue; //Don't duplicate the same byte twice
            }
        }
        */
        
        if (echoChar) //cout << inByte << " ";
            printf("%02X ",inByte);
        
        //accumulate the data in the buffer
        currBuffer.push_back(inByte);
        
        //increment the buffer index for the next time
        curBuffIndex++;
    }//Finish the for loop around the input bytes

    //If there was no data on the wire, then we're done
    if (curBuffIndex==0){
        printf("No data on wire\n");
        return;
    }
    curBuffIndex--; //Decrement to the last entered byte


    //Roll back the pointer to the last end byte seen. Store any extra bytes between the end of the buffer
    //and the last endIdx in the leftoverBytes array.
    int lastPacketEndIdx = curBuffIndex;
    while (currBuffer[lastPacketEndIdx]!=BYTE_END){
        lastPacketEndIdx--;
        
        //And if there's no complete packets in this data
        if (lastPacketEndIdx<0){
            cout << "No complete packets found \n";
            break;
        }
    }
//    printf("So lastPacketEndIdx points to a byte: %2X\n",currBuffer[lastPacketEndIdx]);
    
    //Push the extra data into the leftOVerArray
    leftoverBytes.clear();
    for (int i=lastPacketEndIdx+1; i<currBuffer.size(); ++i) {
        leftoverBytes.push_back(currBuffer[i]);
    }
//    printf("Left %i bytes in the leftover vector\n", leftoverBytes.size());

    //To avoid any compounding buffer issues, we set a max count in the lefover
    //If it's too big we simply clear it
    if (leftoverBytes.size()>MAX_LEFTOVER_SERIAL_BYTES) {
        leftoverBytes.clear();
    }
    
    //Process all the packets we know we have
    //Go forward in the input array, from 0 the last end byte (that we found above)
    int nextIndexToTry = 0;
    int tempResult = 0;
    while (nextIndexToTry<lastPacketEndIdx) {

        if(currBuffer[nextIndexToTry]==BYTE_START){
            //If the message is succesfully created, nextIndexToTry is updated to be
            tempResult = interpretBinaryMessageForward(nextIndexToTry);
            if (tempResult==-1){ //No end byte was seen for that packet and packet's length byte
                nextIndexToTry++;
            }
            else {   //We have just processed a packet successfully
                nextIndexToTry=tempResult;
            }
        }
        else {// if(currBuffer[nextIndexToTry]==BYTE_END){
            nextIndexToTry++;
        }
        
    }
    
    printf("UPDATE END\n");
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
 6-9: Chan0 (32 bit signed int)
 10-13: Chan1 (32 bit signed int
 14: Chan2 (32 bit signed int
 18: Chan3 (32 bit signed int
 22: Chan4 (32 bit signed int
 26: Chan5 (32 bit signed int
 30: Chan6 (32 bit signed int
 34-37: Chan7 (32 bit signed int
 38: End_char 0xC0
 */

int ofxOpenBCI::interpretBinaryMessage(int endInd) {
    
    //assume curBuffIndex has already been incremented to the next open spot
    int startInd = endInd;

    
    //roll backwards to find the start of the packet
    while ((startInd >= 0) && (currBuffer[startInd] != BYTE_START)) {
        startInd--;
    }
    
    //cout << startInd << ", ";
    
    if (startInd < 0) {
        printf("Dropped this packet because it didn't have a start byte\n");
    }
    else if ((endInd - startInd + 1) < 3) {
        // printf("data packet isn't long enough to hold any data...so ignore this data packet\n");
    }
    else {
        
        //Get the number of bytes in the payload. Defined as:
        //Payload Length in Bytes = (1+Nchan)*4= 8*4+4 = 36 bytes
        unsigned char n_bytes = currBuffer[startInd + 1]; //this is the number of bytes in the payload
        
        printf("Expected %i bytes in this packet\n", n_bytes);
        
        // check to see if the payload is at least the minimum length
        if (n_bytes < 4*MIN_PAYLOAD_LEN_INT32) {
            //bad data.  ignore this packet;
            printf("\tAhh it's a runt we have here\n");
        }
        else {
            for (int i = startInd;i<=endInd ; ++i) {
                printf("%02X, ",currBuffer[i]);
            }
            printf("\n");
            //check to see if the payload length matches the measured packet size
            
            if ((startInd + 1 + n_bytes + 1) != endInd) {
                printf("Bad packet: %i, %i. Got: %i \n", startInd, endInd, startInd + 1 + n_bytes + 1);
            }
            else {
                

                int nInt32 = n_bytes / 4;
                
                dataPacket_ADS1299 dataPacket = dataPacket_ADS1299(n_bytes);
                dataPacket.timestamp = time(NULL);
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
 
    printf("\n");
    return startInd;
}

//Rewrote the interpretBinaryMessage fxn to avoid stumbling over a BYTE_END or BYTE_START on accident
int ofxOpenBCI::interpretBinaryMessageForward(int startIdx) {
    
    //assume curBuffIndex has already been incremented to the next starting spot
    int endIdx = startIdx;
    
    unsigned char n_bytes = currBuffer[startIdx + 1]; //this is the number of bytes in the payload
    
    //Counting forward, do we see the BYTE_END?
    if (currBuffer[(startIdx + 1 + n_bytes + 1)] != BYTE_END) {
        printf("Bad packet: %i, %i. Got: %i \n", startIdx, endIdx, startIdx + 1 + n_bytes + 1);
        endIdx = -1;
    }
    else{
        
        endIdx = (startIdx + 1 + n_bytes + 1);
        

        //Prints out the entire message to confirm it's working well
//        for (int i = startIdx;i<=endIdx ; ++i)
//            printf("%02X, ",currBuffer[i]);
//        printf("\n");

        
        int nInt32 = n_bytes / 4;
        
        dataPacket_ADS1299 dataPacket = dataPacket_ADS1299(nInt32);
        dataPacket.timestamp = time(NULL);
        
        dataPacket.sampleIndex = interpretAsInt32(&currBuffer[startIdx+2]); //read the int32 value
        
        //Full doc here: http://www.openbci.com/forums/topic/understanding-serial-interface/
        startIdx += 6;  //increment the start index
        
        int nValToRead = min<int>(nInt32-1,dataPacket.values.size());
        for (int i=0; i < nValToRead;i++) {
            dataPacket.values[i] = interpretAsInt32(&currBuffer[startIdx]); //read the int32 value
            startIdx += 4;  //increment the start index
        }
        outputPacketBuffer.push_back(dataPacket);
    }
    
    
    //printf("\n");
    return endIdx;
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



