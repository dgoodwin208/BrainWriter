#include "openBCI_ADS1299.cpp"
#include <iostream>
#include "TimeoutSerial.h"

int main(int argc, char * argv[])
{
  std::cout << "Testing openBCI_ADS1299 driver \n";
  TimeoutSerial serial(argv[1], 115200);
  unsigned int byteCount = 0;
  bool echoBytes = false;
  openBCI_ADS1299<TimeoutSerial> ads_driver(serial, 8);
  ads_driver.startDataTransfer();
  int curPacketInd = 0;
  while(1)
    {
      ads_driver.read();
      byteCount++;
      if (ads_driver.isNewDataAvailable()) {
	//Get New packet here
	dataPacket_ADS1299 new_packet(8);
	ads_driver.copyDataPacketTo(new_packet);
      }
    }
  std::cout << "Finished Test \n";
  return 0;
}
