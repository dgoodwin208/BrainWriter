
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
/////////////////////////////////////////////////////////////////////////////

#include <string>
#include <algorithm>
#include <time.h>
#import "ofxOpenBCI.h"




//!Fixme there is probably a less lame way to do this!
double millis()
{
    static time_t start = time(0);
    return difftime( time(0), start)*1000.0;
}


  int preffered_datamode;
  int state;
  int dataMode;
  int prevState_millis;

  vector<byte> serialBuff;
  int curBuffIndex;
  dataPacket_ADS1299 dataPacket;
  
  bool isDataPacketUnread;
  int num_channels;
  SerialManager & serial_openBCI;
  unsigned int currBuffIndex;
  //construction


public: openBCI_ADS1299(SerialManager &serial, const int desired_num_channels):
        state(STATE_NOCOM),
								  preffered_datamode(DATAMODE_BIN),
								  dataMode(DATAMODE_BIN), 
								  prevState_millis(0), 
								  currBuffIndex(0),
								  isNewDataPacketAvailable(false), 
								  num_channels(desired_num_channels),
								  dataPacket(desired_num_channels), 
								  serial_openBCI(serial)  { }
  bool isNewDataPacketAvailable;
private:
  int changeState(int newState) {
    state = newState;
    prevState_millis = millis();
    return 0;
  }
  int updateState() {
    if (state == STATE_COMINIT) {
      if ((millis() - prevState_millis) > COM_INIT_MSEC) {

        changeState(STATE_NORMAL);
        startDataTransfer(preffered_datamode);
        //startDataTransfer(DATAMODE_BIN_4CHAN);
      }
    }
    return 0;
  }    

  
  //start the data transfer using the current mode
public: int startDataTransfer() {
    return startDataTransfer(dataMode);
  }
  
  //start data transfer using the given mode
public: int startDataTransfer(int mode) {
    dataMode = mode;
    stopDataTransfer();
    switch (mode) {
      case DATAMODE_BIN:
        serial_openBCI.writeString(command_startBinary + "\n");
        cout << "Processing: OpenBCI_ADS1299: starting binary\n";
        break;
      case DATAMODE_BIN_4CHAN:
        serial_openBCI.writeString(command_startBinary_4chan + "\n");
        cout << "Processing: OpenBCI_ADS1299: starting binary 4-channel\n";
        break;      
      case DATAMODE_TXT:
        serial_openBCI.writeString(command_startText + "\n");
        cout << "Processing: OpenBCI_ADS1299: starting text";
        break;
    }
    return 0;
  }
  
  void stopDataTransfer() {
    serial_openBCI.writeString(command_stop + "\n");
    serial_openBCI.flush(); // clear anything in the com port's buffer
  }
  
  //read from the serial port

  int read(bool echoChar) {
    //get the byte
    byte inByte;
    int inByteInt;
    
    inByteInt = serial_openBCI.readByte();
    if (inByteInt < 0)
      return 0;

    byte(inChar);
    if (echoChar) cout << char(inByte) << " ";
    
    //accumulate the data in the buffer
    serialBuff[curBuffIndex] = inByte;
    
        
    //increment the buffer index for the next time
    curBuffIndex++;     
          
    //is the data packet complete?
    switch (dataMode) {
      case DATAMODE_BIN:
        if (inByte == BYTE_END) interpretBinaryMessage();
        break;
      case DATAMODE_BIN_4CHAN:
        if (inByte == BYTE_END) interpretBinaryMessage();
        break;
      case DATAMODE_TXT:
        if (inByte == CHAR_END) interpretTextMessage();
        break; 
      default:
        //don't accumulate...just reset back to the first place in the buffer
        curBuffIndex=0;
        break;
    }

    //check to make sure that the buffer index hasn't gone too far
    if (curBuffIndex >= serialBuff.size()) curBuffIndex = serialBuff.size() - 1;

    return int(inByte);
  }
  
  //simple public interface for a reading singe char of data from BCI device and updating
  //the datapacket packet as necessary.
  public: int read() {  return read(false); }


  //Accessor for to tell client that a new data packet has been parsed from the byte stream.
public: bool isNewDataAvailable() { return isNewDataPacketAvailable; }

  //activate or deactivate an EEG channel...channel counting is zero through nchan-1
  public: void changeChannelState(int Ichan,bool activate) {
    if (serial_openBCI.isOpen()) {
      if ((Ichan >= 0) && (Ichan < command_activate_channel.size())) {
        if (activate) {
          serial_openBCI.write(command_activate_channel[Ichan] + "\n");
        } else {
          serial_openBCI.write(command_deactivate_channel[Ichan] + "\n");
        }
      }
    }
  }
  
  //deactivate an EEG channel...channel counting is zero through nchan-1
  public: void deactivateChannel(int Ichan) {
    if (serial_openBCI.isOpen()) {
      if ((Ichan >= 0) && (Ichan < command_activate_channel.size())) {
        serial_openBCI.write(command_activate_channel[Ichan]);
      }
    }
  }

  //return the state
  bool isStateNormal() { 
    if (state == STATE_NORMAL) { 
      return true;
    } else {
      return false;
    }
  }
  
  //interpret the data
  int interpretBinaryMessage() {
    //assume curBuffIndex has already been incremented to the next open spot
    int startInd = curBuffIndex-1;
    int endInd = curBuffIndex-1;
    
    
     
    //roll backwards to find the start of the packet
    while ((startInd >= 0) && (serialBuff[startInd] != BYTE_START)) {
      startInd--;
    }
    if (startInd < 0) {
      //didn't find the start byte..so ignore this data packet
    } else if ((endInd - startInd + 1) < 3) {
      //data packet isn't long enough to hold any data...so ignore this data packet
    } else {
      int n_bytes = int(serialBuff[startInd + 1]); //this is the number of bytes in the payload
      
      
      // check to see if the payload is at least the minimum length
      if (n_bytes < 4*MIN_PAYLOAD_LEN_INT32) {
        //bad data.  ignore this packet;
      } else {
        //check to see if the payload length matches the measured packet size
        if ((startInd + 1 + n_bytes + 1) != endInd) {
          //bad data.  ignore this packet
        } else {
          
          int startIndPayload = startInd+1+1;
          int nInt32 = n_bytes / 4;
          interpretBinaryPayload(startIndPayload,nInt32);
          //dataPacket.printToConsole();
        }
      }      
    }
    
    curBuffIndex=0;  //reset buffer counter back to zero to start refilling the buffer
    return 0;
  }
  int interpretBinaryPayload(int startInd,int nInt32) {
    dataPacket.sampleIndex = interpretAsInt32(&serialBuff[startInd]); //read the int32 value
    startInd += 4;  //increment the start index
    
    int nValToRead = min<int>(nInt32-1,dataPacket.values.size());
    for (int i=0; i < nValToRead;i++) {
      dataPacket.values[i] = interpretAsInt32(&serialBuff[startInd]); //read the int32 value
      startInd += 4;  //increment the start index
    }
    
    isNewDataPacketAvailable = true;
    return 0;
  }
  int interpretAsInt32(byte byteArray[]) {
    //big endian
//    return int(
//      ((0xFF & byteArray[0]) << 24) | 
//      ((0xFF & byteArray[1]) << 16) |
//      ((0xFF & byteArray[2]) << 8) | 
//      (0xFF & byteArray[3])
//      );
      
    //little endian
    return int(
      ((0xFF & byteArray[3]) << 24) | 
      ((0xFF & byteArray[2]) << 16) |
      ((0xFF & byteArray[1]) << 8) | 
      (0xFF & byteArray[0])
      );
  }
  
  int interpretTextMessage() {
    //still have to code this!
    curBuffIndex=0;  //reset buffer counter back to zero to start refilling the buffer
    return 0;
  }
  
  int copyDataPacketTo(dataPacket_ADS1299 &target) {
    isNewDataPacketAvailable = false;
    dataPacket.copyTo(target);
    return 0;    
  }



