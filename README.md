This is a collection of code for the BrainWriter project. Currently, it's just the ofxOpenBCI OpenFrameworks add-on and a testing app (in development), but there will also be the data server and associated webpage coming soon.

The Betamaker OpenFrameworks project is for eliciting beta waves and storing the data in a way that can be used down the road


This is a library to link the OpenBCI EEG acquisition device to OpenFrameworks.

This library has a few co-dependcies:
ofxHistoryPlot (for displaying a time plot)
ofxOpenBCI (attached in the src directory)
ofxIO (supporting ofxSerial)
ofxSerial: the serial library used to manage the serial port connection

Note:
+ Once the app has started, the user has to wait 3-5 seconds then press the 'b' key to start streaming data from the device.
+ This will store a log file on the desktop. The "~/" notation is hardcoded, so this would likely cause issues if ran on a windows machine.
