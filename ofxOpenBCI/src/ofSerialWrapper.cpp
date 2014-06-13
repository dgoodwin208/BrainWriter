/* Author: Jon Weisz
 *
 * This class creates a serial port connection that provides a minimally
 * necessary subset of the functions needed by the openBCI_ADS1299 driver
 * class so that it can serve as a drop in replacement to the
 * serial communication managers from 
 * https://gitorious.org/serial-port/serial-port/
 */




class ofSerialWrapper 
{
private:
  SerialManager sm_;
  ofSerial & ofs;

public: 
  ofSerialWrapper(ofSerial & serial) : ofs(serial)
  void close()
  {
    ofs.close()
  };
  void flush()
  {
    ofs.flush();
  };
  void write(const char *data, size_t size);
  {
    ofs.writeByes(data, size);
  }

  void writeString(const std::string & s)
  {
    write(s.c_str(), s.size());
  }

  void read(char *data, size_t size);
  {
    ofs.readBytes(data, size);
  }
  
  int readByte()
  {
    return ofs.readByte();
  }

  bool isOpen()
  {
    return ofs.isAvailable();
  };
}; 
