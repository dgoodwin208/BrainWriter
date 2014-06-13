

class openBCI_ADS1299 {
  int preffered_datamode;
  int state;
  int dataMode;
  int prevState_millis;

  std::vector<byte> serialBuff;
  int curBuffIndex;
  dataPacket_ADS1299 dataPacket;
  
  bool isDataPacketUnread;
  int num_channels;
  SerialManager & serial_openBCI;
  unsigned int currBuffIndex;
  //construction


public:
    openBCI_ADS1299(SerialManager &serial,
			const int desired_num_channels): state(STATE_NOCOM), 
								  preffered_datamode(DATAMODE_BIN),
								  dataMode(DATAMODE_BIN), 
								  prevState_millis(0), 
								  currBuffIndex(0),
								  isNewDataPacketAvailable(false), 
								  num_channels(desired_num_channels),
								  dataPacket(desired_num_channels), 
								  serial_openBCI(serial)  { }
  bool isNewDataPacketAvailable;

private: int changeState(int newState);
    int updateState();

  
  //start the data transfer using the current mode
public: int startDataTransfer();
  
  //start data transfer using the given mode
public: int startDataTransfer(int mode);
  
    void stopDataTransfer();
  
  //read from the serial port

    int read(bool echoChar);
  
  //simple public interface for a reading singe char of data from BCI device and updating
  //the datapacket packet as necessary.
  public: int read() {  return read(false); }


  //Accessor for to tell client that a new data packet has been parsed from the byte stream.
public: bool isNewDataAvailable();

  //activate or deactivate an EEG channel...channel counting is zero through nchan-1
public: void changeChannelState(int Ichan,bool activate);
  
  //deactivate an EEG channel...channel counting is zero through nchan-1
public: void deactivateChannel(int Ichan);
  //return the state
  bool isStateNormal() { 
    if (state == STATE_NORMAL) { 
      return true;
    } else {
      return false;
    }
  }
  
    //interpret the data
    int interpretBinaryMessage();
    
    int interpretBinaryPayload(int startInd,int nInt32);
    
    int interpretAsInt32(byte byteArray[]);
  
    int interpretTextMessage();
  
    int copyDataPacketTo(dataPacket_ADS1299 &target);


};

