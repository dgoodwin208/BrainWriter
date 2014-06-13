This is a library to link the OpenBCI EEG acquisition device to OpenFrameworks. 

This library has a few co-dependcies:
ofxHistoryPlot (for displaying a time plot)
ofxOpenBCI (attached in the src directory)
ofxIO (supporting ofxSerial)
ofxSerial: the serial library used to manage the serial port connection

Note: 
+ Once the sample app has started, the user has to wait 3-5 seconds then press the 'b' key to start streaming data from the device.
+ This will store a log file on the desktop. The "~/" notation is hardcoded in the sample code, so this would likely cause issues if ran on a windows machine.

Credit: 
Props to Jon Weisz for his work writing a c++ wrapper for OpenBCI, which was adapted to work in the OpenFrameworks context 
