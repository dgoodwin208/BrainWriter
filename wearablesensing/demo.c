/*

# This file is part of the Application Programmer's Interface (API) for Dry
# Sensor Interface (DSI) EEG systems by Wearable Sensing. The API consists of
# code, headers, dynamic libraries and documentation.  The API allows software
# developers to interface directly with DSI systems to control and to acquire
# data from them.
# 
# The API is not certified to any specific standard. It is not intended for
# clinical use. The API, and software that makes use of it, should not be used
# for diagnostic or other clinical purposes.  The API is intended for research
# use and is provided on an "AS IS" basis.  WEARABLE SENSING, INCLUDING ITS
# SUBSIDIARIES, DISCLAIMS ANY AND ALL WARRANTIES EXPRESSED, STATUTORY OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO ANY IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT OR THIRD
# PARTY RIGHTS.
# 
# Copyright (c) 2014-2015 Wearable Sensing LLC

This file illustrates some of the functionality of the DSI C/C++ API.
It is intended to be read as documentation with examples, but it can be compiled.
It does not detail every function of the API.  See DSI.h for the full list of
functions.

1. Getting started

To create a DSI application:

(1) Add the file DSI_API_Loader.c to your project (can be compiled as C or C++)

(2) #include "DSI.h" in your own C or C++ code

(3) Call Load_DSI_API() in your code before calling any of the other functions.
    The argument to Load_DSI_API may be a string that explicitly specifies the
    name of the dynamic library file to load. Alternatively, if you pass NULL
    then the default name will be used.  This default can be manipulated at
    compile time by defining the DSI_PLATFORM symbol. For example, using gcc on
    Mac OSX on a 64-bit intel machine:

       gcc -DDSI_PLATFORM=-Darwin-x86_64  demo.c DSI_API_Loader.c

    will cause Load_DSI_API(NULL) to look for "libDSI-Darwin-x86_64.dylib"
    instead of just "libDSI.dylib". As another example, if you're using Visual
    C++ to compile a 32-bit binary on a Windows machine, you could say:

       cl /DDSI_PLATFORM=-Windows-i386  demo.c DSI_API_Loader.c

    which would cause Load_DSI_API(NULL) to look for "libDSI-Windows-i386.dll"
    instead of just "libDSI.dll" .

(4) Check the return value of Load_DSI_API before proceeding. It will be zero on
    success. Any non-zero value means the API failed to load and other API calls
    (function names starting with DSI_) should not be called.

    If the return value is negative, it means that the libDSI file could not be
    located or opened. If this is the case, check the following rules for locating
    dynamic libraries at runtime:

    * On Windows, the .dll file needs to be either in the current working directory,
      or in one of the directories listed (separated by semicolons) in the Path
      environment variable.
      
    * On Mac OSX, the .dylib file needs to be either in the current working
      directory, or in one of the directories listed (separated by colons) in the
      DYLD_LIBRARY_PATH or LD_LIBRARY_PATH environment variables.
      
    * On Linux, the .so file must be in one of the directories listed (separated by
      colons) in the LD_LIBRARY_PATH environment variable (the current working
      directory will not be searched by default). Note that use on Linux is only
      partially supported. You mileage will likely vary in attempting to pair the
      headset via Bluetooth and create a suitably-configured serial-port device.
      Currently, your system's version of GLIBC (the GNU C Library shared object)
      must be 2.14 or later.


This file may be compiled. In it, we will build documented example applications
for recording signal or impedance values to a .csv file.  The `main()` function,
below, illustrates how to load the API and check for errors.  It will call the
impedance demo if compiled with the preprocessor macro IMPEDANCES defined as 1
(`gcc -DIMPEDANCES=1 ....` or `cl.exe /DIMPEDANCES=1 ...`). Otherwise it will
call the signal recording demo.
                                                                              */

#include "DSI.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#ifndef IMPEDANCES
#define IMPEDANCES 0
#endif

    
    // These demo and test functions will be defined in due course.
    int    SignalDemo( const char * serialPort, const char * montage, const char * reference );
    int ImpedanceDemo( const char * serialPort, const char * montage, const char * reference );
    int   MontageDemo( const char * serialPort );
    int NameTest( DSI_Headset h, const char * namingScheme );

    
    int main( int argc, const char * argv[] )
    {

        const char * port       = ( argc > 1 && *argv[ 1 ] ) ? argv[ 1 ] : NULL;
        // First command-line argument: serial port address (e.g. "COM4" on
        // Windows, or "/dev/cu.DSI24-004-BluetoothSeri" on OSX)
        
        const char * montage    = ( argc > 2 && *argv[ 2 ] ) ? argv[ 2 ] : NULL;
        // Optional second command-line argument: a space-delimited (so enclose
        // it in quotes) list of channel specifications. Use the empty string
        // to stick to the default.
        
        const char * reference  = ( argc > 3 && *argv[ 3 ] ) ? argv[ 3 ] : NULL;
        // Optional third command-line argument: the name of sensor (or linear
        // combination of sensors, without spaces) to be used as reference.


        const char * dllname = NULL;
        // NULL causes the default name for the dynamic library to be used
        // (incorporating the DSI_PLATFORM macro, if defined).
        
        int load_error = Load_DSI_API( dllname );
        if( load_error < 0 ) return fprintf( stderr, "failed to load dynamic library \"%s\"\n", DSI_DYLIB_NAME( dllname ) );
        if( load_error > 0 ) return fprintf( stderr, "failed to import %d functions from dynamic library \"%s\"\n", load_error, DSI_DYLIB_NAME( dllname ) );
        
        fprintf( stderr, "DSI API Version %s loaded\n", DSI_GetAPIVersion() );

        if( IMPEDANCES ) // compile with -DIMPEDANCES=1 (or /DIMPEDANCES=1 on MSVC) to compile the impedance demo 
        {
            if( argc < 2 ) return fprintf( stderr,
                "\n"
                "    %s  SERIALPORT [ MONTAGE [ REFERENCE ] ]\n"
                "\n"
                "Example:\n"
                "\n"
                "    %s  COM4  >  impedances.csv\n"
                "\n"
                "Connects to COM4, waits for a second to allow impedance\n"
                "values to settle, then records impedances for a few\n"
                "seconds, dumping the output to impedances.csv\n"
                "(before you average them, plot the values to ensure\n"
                "that they really have reached a plateau).\n"
                "\n",
                argv[ 0 ], argv[ 0 ] );
                
            return ImpedanceDemo( port, montage, reference );
        }
        else
        {
            if( argc < 2 ) return fprintf( stderr,
                "\n"
                "    %s  SERIALPORT [ MONTAGE [ REFERENCE ] ]\n"
                "\n"
                "Example:\n"
                "\n"
                "    %s  COM4  \"C3 C4\"  Pz  >  signals.csv\n"
                "\n"
                "Connects to COM4 and records two-channel EEG data re-referenced\n"
                "to Pz, for a few seconds, dumping output to signal.csv\n"
                "\n",
                argv[ 0 ], argv[ 0 ] );
                
            return SignalDemo( port, montage, reference );
        }
    }
                                                                              /*


2. Data types in the DSI API


2.1 `DSI_Headset`

A pointer representing a single DSI headset. Typically we will use the letter
`h` to denote pointers of this type. The API provides various functions whose
names start with `DSI_Headset_`: these take a `DSI_Headset` pointer as their
first argument and can be considered "methods" of the `DSI_Headset` object.
A `DSI_Headset` pointer is created using `DSI_Headset_New( port );` (pass
NULL as the port if you want to defer opening the serial-port connection)
and disposed of using `DSI_Headset_Delete( h );`


2.2 `DSI_Channel`

A pointer representing a single DSI channel. Typically we will use the letter
`c` to denote pointers of this type. The API provides various functions whose
names start with `DSI_Channel_`: these take a `DSI_Channel` pointer as their
first argument and can be considered "methods" of the `DSI_Channel` object.  The
following code (an example of a `DSI_SampleCallback` function) illustrates one
way in which `DSI_Channel` pointers can be acquired and used:
                                                                              */
    void PrintSignals( DSI_Headset h, double packetOffsetTime, void * userData )
    {
        unsigned int channelIndex;
        unsigned int numberOfChannels = DSI_Headset_GetNumberOfChannels( h );
        
        // This function uses `userData` as nothing more than a crude boolean
        // flag: when it is non-zero, we'll print headings; when it is zero,
        // we'll print signal values
        
        if( userData ) printf( "%11s",    "Time" );
        else           printf( "% 11.4f", packetOffsetTime );
        
        for( channelIndex = 0; channelIndex < numberOfChannels; channelIndex++ )
        {
            DSI_Channel c = DSI_Headset_GetChannelByIndex( h, channelIndex );
            
            if( userData ) printf( ",%11s",    DSI_Channel_GetString( c ) );
            else           printf( ",% 11.4f", DSI_Channel_GetSignal( c ) );
        }
        printf( "\n" );
        
    }
                                                                             /*

2.3 `DSI_Source`

A pointer representing a single DSI "source". Typically we will use the letter
`s` to denote pointers of this type. The API provides various functions whose
names start with `DSI_Source_`: these take a `DSI_Source` pointer as their first
argument and can be considered "methods" of the `DSI_Source` object. The
difference between a "source" and a "channel" is subtle, and is explained in the
next subsection.  Usually, you will interact directly with `DSI_Source` pointers
only when measuring impedance or other measures of sensor contact quality. For
anything else, you will probably want to deal with `DSI_Channel` pointers.
The following code (an example of a `DSI_SampleCallback` function) illustrates
one way in which `DSI_Source` pointers can be acquired and used:
                                                                              */
    void PrintImpedances( DSI_Headset h, double packetOffsetTime, void * userData )
    {
        unsigned int sourceIndex;
        unsigned int numberOfSources = DSI_Headset_GetNumberOfSources( h );
        
        // This function uses `userData` as nothing more than a crude boolean
        // flag: when it is non-zero, we'll print headings; when it is zero,
        // we'll print impedance values.
        
        if( userData ) printf( "%9s",    "Time" );
        else           printf( "% 9.4f", packetOffsetTime );
        
        for( sourceIndex = 0; sourceIndex < numberOfSources; sourceIndex++ )
        {
            DSI_Source s = DSI_Headset_GetSourceByIndex( h, sourceIndex );
            
            if( DSI_Source_IsReferentialEEG( s ) && ! DSI_Source_IsFactoryReference( s ) )
            {
                if( userData ) printf( ",%9s",    DSI_Source_GetName( s ) );
                else           printf( ",% 9.4f", DSI_Source_GetImpedanceEEG( s ) );
            }
        }
        
        // The common-mode follower (CMF) sensor, at the factory reference position,
        // is a special case:
        
        if( userData ) fprintf( stdout, ",   CMF=%s\n", DSI_Headset_GetFactoryReferenceString( h ) );
        else           fprintf( stdout, ",% 9.4f\n",    DSI_Headset_GetImpedanceCMF( h ) );
    }
                                                                              /*
2.3.1  The distinction between a "Channel" and a "Source"

Most of your code for processing, visualizing and storing signals will want to
interact with `DSI_Channel` pointers. These objects deliver, and automatically
buffer, signals that can be flexibly configured and reordered. Their positive
and negative components (signal and reference) can be explicitly specified,
thereby making re-referencing easy.  This means that a `DSI_Channel` may
be influenced by the activity of more than one EEG sensor.

For this reason, when you measure impedances or other measures of sensor contact
quality, or when you specify the individual positive and negative components
that constitute a Channel, you're dealing at a lower level with a different
kind of object, the `DSI_Source`.

A `DSI_Source` is a lower-level object representing one of the signals that the
headset firmware delivers natively.  Some Sources will correspond to referential
EEG sensors; others might correspond to auxiliary bipolar sensors, to the
digital trigger channel, or to virtual signals such as clock signals or the
zero-volt flat line that represents the factory reference sensor itself.  If a
Source corresponds to a referential EEG sensor, it will deliver a signal
referenced to the factory reference sensor, which is typically Pz:  as a result,
the raw EEG signal from a Source would look somewhat unusual (you might see
alpha waves from the visual cortex even at frontal locations, for example).

When you first initialize a `DSI_Headset`,  the API will attempt to find a more
"traditional" reference:  first it will look for a linked-ear reference, which
on some headset models is delivered as a single Source `A1A2`; on others it is
obtained by averaging the signal from two Sources, `A1` and `A2`.  Failing that,
it will look for a linked-mastoid reference (`M1M2`, or the average of `M1` and
`M2`, depending on headset model). If the headset finds one of these
possibilities, it will set the default reference accordingly---the default set
of `DSI_Channel` pointers will then deliver ear- or mastoid-referenced EEG. If
it fails to find a record of ear or mastoid sensors in the headset's flash
memory, it will issue a warning (a message with a debug level of 1, which you
can receive if you have registered a `DSI_MessageCallback`) and the factory
reference sensor (usually Pz) will remain as the default reference.  The default
reference can be queried with `DSI_Headset_GetReferenceString(h)`.   If your
headset is equipped with ear or mastoid electrodes, but these have not been
recognized, then you may need to fix the content of the headset's flash memory:
contact the manufacturer for instructions.


2.4 `DSI_ProcessingStage`

A pointer representing a custom processing stage. Typically we will use the
letter `p` to denote pointers of this type. The API provides various functions
whose names start with `DSI_ProcessingStage_`: these take a
`DSI_ProcessingStage` pointer as their first argument and can be considered
"methods" of the `DSI_ProcessingStage` object.  ProcessingStages are optional.
They allow the signal to be transformed automatically in arbitrary ways before
it is finally delivered to your application. They can be created using the
headset method `DSI_Headset_AddProcessingStage`.  Processing is carried
out whenever a new sample becomes available, and is performed in a custom
`DSI_SampleCallback` function, written by you. A demo is provided below: 
                                                                              */
#include <math.h>
#ifndef CUSTOM_PROCESSING
#define CUSTOM_PROCESSING 0
#endif
// Compile with `gcc -DCUSTOM_PROCESSING=1 ....` or `cl.exe /DCUSTOM_PROCESSING=1 ...`
// to enable this optional feature of the SignalDemo    

    void CustomProcessingDemo( DSI_Headset h, double t, void * userData )
    {
        unsigned int numberOfChannelsIn, numberOfChannelsOut, channelIndex;
        double cycle, freq;
        void * paramData;

        DSI_ProcessingStage output = ( DSI_ProcessingStage )userData;
        // The userData will always point to provide a pointer to the
        // previous DSI_ProcessingStage in the chain.
        
        DSI_ProcessingStage input = DSI_ProcessingStage_GetInput( output );
        if( !input || !output ) return;
        
        paramData = DSI_ProcessingStage_ParamData( output );
        if( paramData ) freq = *( double * )paramData;
        else freq = 2.0;
        
        cycle = sin( 2.0 * 3.14159265358979323846 * freq * t );
        // This example will, somewhat pointlessly, inject periodic zeros
        // into the signal just to provide obvious visible proof that
        // insertion of a DSI_ProcessingStage has worked.
        
        numberOfChannelsIn  = DSI_ProcessingStage_GetNumberOfChannels( input );
        numberOfChannelsOut = DSI_ProcessingStage_GetNumberOfChannels( output );
        // If the output processing stage was not explicitly reconfigured with
        // DSI_ProcessingStage_ClearChannels() and DSI_ProcessingStage_AddChannel(),
        // it will have the same number of channels as its input.  Let's assume
        // a 1:1 mapping between input and output channels for the current demo.
        
        for( channelIndex = 0; channelIndex < numberOfChannelsIn && channelIndex < numberOfChannelsOut; channelIndex++ )
        {
            double signal = DSI_ProcessingStage_Read( input, channelIndex, 0 );
            // Sample 0 is the most recent, 1 the previous sample, and so on.
            
            if( cycle > 0.9 ) signal = 0.0;
            // This puts periodic holes in the signal, just to show that it's working.
            // Your implementation would presumably do something more meaningful,
            // like filtering or artifact removal.
            
            DSI_ProcessingStage_Write( output, channelIndex, signal );
            // This pushes one sample into the buffer of the specified channel
            // in the output ProcessingStage.  Make sure you the same number
            // of samples to every channel.
            
        }
    }
                                                                              /*


2.5 `DSI_SampleCallback`

A pointer to a function implemented by you, which has the following prototype:

    void func( DSI_Headset h, double packetTime, void * userData );

A function of this type can be registered using 
`DSI_Headset_SetSampleCallback(h, func, userData);`  This causes func to be
called whenever a new sample of signal data becomes available during a call to
`DSI_Headset_Idle()` or `DSI_Headset_Receive()`.  The `packetTime` argument will
contain the time-stamp of the sample in seconds, and `userData` contains
whichever pointer you supplied during the call to
`DSI_Headset_SetSampleCallback`.  Both `PrintSignals()` and `PrintImpedances()`,
above, are examples of `DSI_SampleCallback` functions.


2.6 `DSI_MessageCallback`

A pointer to a function implemented by you, which has the following prototype:

    int func( const char * msg, int debugLevel );

A function of this type can be registered using
`DSI_Headset_SetMessageCallback(h, func);` This causes func to be called
whenever the API delivers a warning or debugging message during a call to
`DSI_Headset_Idle()` or `DSI_Headset_Receive()`. We will use a simple example:
                                                                              */
    int Message( const char * msg, int debugLevel )
    {
        return fprintf( stderr, "DSI Message (level %d): %s\n", debugLevel, msg );
    }
                                                                              /*
The `debugLevel` argument will contain a number that indicates the urgency or
criticality of the message: higher numbers indicate more trivial or routine
messages. You can use `DSI_Headset_SetVerbosity( h, debugLevel )` to suppress
messages whose level is higher than `debugLevel` (the default level is 2).

A `DSI_MessageCallback` function can also be registered globally using
`DSI_SetErrorCallback( func );`  This provides the opportunity to handle errors
in a more thread-safe way than with `DSI_Error()`, as described in the next
section.


3. Handling API errors

There are two ways to handle errors in the DSI API. Whenever a DSI API call
throws an error, it will store a single global string pointer that contains the
error message.  Both `DSI_Error()` and `DSI_ClearError()` can be used to check
for errors: they both return `NULL` if no error message has been issued since
the last call to `DSI_ClearError()`.  If there has been an error, they return a
`const char *`  to the error message. Note that all `const char *`  string
pointers returned by DSI API functions like this are semi-persistent and
non-thread-safe.  If you need to keep the string, allocate memory and copy it
(the easy way is to assign it to a `std::string` in C++) before calling any more
DSI API functions.

You should check for errors after any DSI API call.  One way to do this is
illustrated in the following example, where a macro `CHECK` is defined to allow
easy return from `main()` or from any other function that announces failure
using a non-zero return value:
                                                                              */
    int CheckError( void )
    {
        if( DSI_Error() ) return fprintf( stderr, "%s\n", DSI_ClearError() );
        else return 0;
    }
    #define CHECK     if( CheckError() != 0 ) return -1;
                                                                              /*
We will use the `CHECK` macro to make our subsequent examples more readable, but
it is up to you to define it, or something similar, if you choose to handle
errors this way in your own code.

Reliance on a single global error pointer will be sufficient for most purposes,
but may cause problems in some multi-threaded designs.  An alternative method is
to register a `DSI_MessageCallback` function using the global function
`DSI_SetErrorCallback( func )`.   Again, this approach makes use of global
memory to store the function pointer---so separate threads should not attempt to
set *different* error callbacks---but the error string itself will not be global
when passed to the callback, and will not conflict with errors issued by other
threads. Your implementation of the callback can then implement mutexing as
necessary,  associate an error notification with the current thread id, or
whatever other measures are appropriate for your design.


4. Application structure

Your application's main loop will most likely center around the function
`DSI_Headset_Idle(h, seconds);`  This processes any events and data samples
that are already pending (calling your function, if you registered it with
`DSI_Headset_SetSampleCallback`, on every new sample).  It then continues
processing for the specified number of seconds. Its use is illustrated in
the `SignalDemo` example, as follows:
                                                                              */
                                                                              
#ifndef VERBOSITY
#define VERBOSITY 2
#endif

    int SignalDemo( const char * serialPort, const char * montage, const char * reference )
    {
        DSI_Headset h;
        DSI_ProcessingStage p;
        double deadline;
        
        h = DSI_Headset_New( NULL ); CHECK
        // Passing NULL defers setup of the serial port connection until later.
        
        DSI_Headset_SetMessageCallback( h, Message ); CHECK
        DSI_Headset_SetVerbosity( h, VERBOSITY ); CHECK
        // This will allows us to configure the way we handle any debugging messages
        // that occur during connection.
        
        DSI_Headset_Connect( h, serialPort ); CHECK
        // This establishes the serial port connection and initializes the
        // headset.
        
        DSI_Headset_ChooseChannels( h, montage, reference, 1 ); CHECK
        
        fprintf( stderr, "%s\n", DSI_Headset_GetInfoString( h ) ); CHECK
        
        PrintSignals( h, 0, "headings" ); CHECK
        DSI_Headset_SetSampleCallback( h, PrintSignals, NULL ); CHECK
        // This registers our DSI_SampleCallback, to be called on every
        // new sample during DSI_Headset_Idle.
        
        
        if( CUSTOM_PROCESSING ) // Compile with `gcc -DCUSTOM_PROCESSING=1 ....` or with
        {                       // `cl.exe /DCUSTOM_PROCESSING=1 ...`  to enable this optional feature
        
           double interruptionFrequencyHz = CUSTOM_PROCESSING;
           // let the numeric value of the macro dictate the parameter for our "periodic zeros" demo.
           p = DSI_Headset_AddProcessingStage( h, "test", CustomProcessingDemo, &interruptionFrequencyHz, NULL ); CHECK
        }

        
        DSI_Headset_StartDataAcquisition( h ); CHECK
        // This starts the sample-by-sample flow of data from the headset.
        
        deadline = DSI_Headset_SecondsSinceConnection( h ) + 5.0;
        // For the purposes of this demo we'll be recording for only 5 seconds.
        
        while( DSI_Headset_SecondsSinceConnection( h ) < deadline )
        {
            DSI_Headset_Idle( h, 0.0 ); CHECK
            
            // Do other application-defined things here as necessary.
            // For example, if this is a real-time application relying
            // on smooth signal delivery, read the buffered signal at
            // regularly-timed intervals here, instead of using a
            // SampleCallback function to respond to each new sample.
        } 
        DSI_Headset_SetSampleCallback( h, NULL, NULL ); CHECK
        DSI_Headset_StopDataAcquisition( h ); CHECK
        DSI_Headset_Idle( h, 1.0 ); CHECK
        DSI_Headset_Delete( h ); CHECK
        
        return 0;
    }
                                                                              /*
Our second demo illustrates how we would go about setting up the headset
to measure impedances:
                                                                              */
    int ImpedanceDemo( const char * serialPort, const char * montage, const char * reference )
    {
        DSI_Headset h;
        
        h = DSI_Headset_New( NULL ); CHECK
        // Passing NULL defers setup of the serial port connection until later.
        
        DSI_Headset_SetMessageCallback( h, Message ); CHECK
        DSI_Headset_SetVerbosity( h, VERBOSITY ); CHECK
        // This will allows us to configure the way we handle any debugging messages
        // that occur during connection.
        
        DSI_Headset_Connect( h, serialPort ); CHECK
        // This establishes the serial port connection and initializes the
        // headset.
        
        DSI_Headset_ChooseChannels( h, montage, reference, 1 ); CHECK

        DSI_Headset_StartImpedanceDriver( h ); CHECK
        // The impedance driver injects current at 110Hz and 130Hz, to
        // allow impedances to be measured. It is off by default when
        // you initialize the headset.
        
        fprintf( stderr, "%s\n", DSI_Headset_GetInfoString( h ) ); CHECK

        DSI_Headset_StartDataAcquisition( h ); CHECK
        // This starts the sample-by-sample flow of data from the headset.
        
        DSI_Headset_Idle( h, 1.0 ); CHECK
        // Let's not print impedances until they have settled for a second.
        
        PrintImpedances( h, 0, "headings" ); CHECK
        DSI_Headset_SetSampleCallback( h, PrintImpedances, NULL ); CHECK
        // This registers the callback we defined earlier, ensuring that
        // impedances are printed to stdout every time a new sample arrives
        // during DSI_Headset_Idle() or DSI_Headset_Receive().
        
        DSI_Headset_Receive( h, 5.0, 1.0 ); CHECK
        // This is a shortcut: it turns on data acquisition mode if it is
        // not already on, then processes events for 5 seconds---the same
        // as DSI_Headset_Idle( h, 5.0 )---or until data acquisition is
        // stops. It then ensures data acquisition is turned off and
        // processes events for a further 1 second. (A negative value would
        // mean "process forever").  Typically, you will want to
        // use DSI_Headset_Idle( h, 0) inside your own main loop instead of
        // a single call to DSI_Headset_Receive().
        
        DSI_Headset_Delete( h ); CHECK
        
        return 0;
    }
                                                                              /*

5. Buffering

Our example `PrintSignals`, above, demonstrates the simplest way of accessing
EEG signals, with `DSI_Channel_GetSignal(c)`.  This returns the value of the
most recently acquired sample, independent of any buffering, and you can then
process this as you see fit.  The logical place to do so is in a
`DSI_SampleCallback` function that you implement and register using
`DSI_Headset_SetSampleCallback`: your function will then be called whenever a
new sample becomes available. For real-time applications, one very common way to
process a new sample is buffer it, to smooth the delivery of the signal with
respect to time. You can set up your own mechanism for buffering, or take
advantage of the buffering already automatically provided by every
`DSI_Channel`. By default, each `DSI_Channel` provides a 1-second buffer, in
which all newly acquired samples are stored. The best way to use this is to call
`DSI_Channel_ReadBuffered(c) at regularly timed intervals during your
application's main loop to ensure smooth delivery, as indicated in the comments
of `SignalDemo()` above. This approach comes at the inevitable cost of forcing
you to wait for the buffer to be filled, and hence introducing some group delay.

You can use `DSI_Channel_GetNumberOfBufferedSamples(c)` or
`DSI_Headset_GetNumberOfBufferedSamples(h)` to query how many samples have been
accumulated at any one time. `DSI_Channel_ReadBuffered(c)` reads the oldest
sample in a DSI_Channel's buffer AND REMOVES IT from the buffer (so you should
read the same number of times from every DSI_Channel, to keep them in step). You
can use `DSI_Channel_GetNumberOfOverflowedSamples(c)` or
`DSI_Headset_GetNumberOfOverflowedSamples(h)` to find out how many have
overflowed, so if there is any danger of your reads failing to keep up with the
flow, you should check that. Overflows will not cause an error or warning unless
you choose to issue it yourself.  In other words, buffering is available for
your convenience but it is optional.

Finally, you can also peek into the buffer with `DSI_Channel_Lookback(c, nSamples)`.
Provided the buffer contains enough samples, you can look back `nSamples` without
anything being removed from the buffer.  In fact, `DSI_Channel_GetSignal(c)` is
exactly the same as `DSI_Channel_Lookback(c, 0)`.


6.  Advanced topics

6.1  Specifying a montage and reference

When it first connects, the headset will choose a default montage and a
traditional reference (see section 2.3.1). However, you can change the
number, order and referencing of the channels yourself. The function
`DSI_Headset_ChooseChannels( h, montage, defaultReference, autoswap )` allows
you to change the montage and/or reference. The string `montage` is a comma- or
space-delimited list of channel specifications. Each channel specification may
be the name of a sensor, or it may be an explicit specification of the channel
and its reference (without spaces). For example, the following is a legal
montage string that illustrates a number of different channel behaviors:
"F3    C3-F3    C4-F4   Pz-P3/2-P4/2    TRG".  The only referential EEG sensor
for which it does not explicitly specify a reference is the first channel, F3,
which will therefore be referenced to the "default" reference.  The default
reference may be changed by supplying a non-empty third argument to
`DSI_Headset_ChooseChannels`, or by calling `DSI_Headset_SetDefaultReference(h,
ref, autoswap)`. It may be queried with `DSI_Headset_GetReferenceString(h)`.

The final boolean argument to `DSI_Headset_SetDefaultReference()`,
`DSI_Headset_SetTraditionalReference()` or `DSI_Headset_ChooseChannels()` is the
"autoswap" option.  If enabled, this means that if your montage starts out as,
say, "Cz M1 M2" with default reference "Pz", and you re-reference that to
"M1/2+M2/2",  the first of the redundant channels gets replaced by the old
reference sensor, i.e. in this example you end up with "Cz Pz M2" with default
reference "M1/2+M2/2",  instead of the redundant "Cz M1 M2".

A montage may also be constructed one channel at a time: start with
`DSI_Headset_ForgetMontage(h)`, then call `DSI_Headset_SetDefaultReference( h,
ref, 0 );`   (with an empty montage, the `autoswap` option will do nothing).
Then use a call like `DSI_Headset_AddChannelToMontage_FromString(h, "Pz-F3", 0);`
to a channel created from a single specification string. Each channel spec
should not contain spaces, nor any punctuation except /+-*, and must consist of
recognized Source IDs ("Sources" are discussed in the section 2.3). If you're
unlucky enough that your headset contains no Source ID information in its flash
memory, then (a) contact the manufacturer and (b) you can still refer to the
Sources using strings like "SRC01", "SRC02" or even just "1", "2", etc. You can
also `DSI_Headset_AddChannelToMontage_FromSource(h, s)`, where `s` is a
`DSI_Source` pointer (see section 2.3).

The `MontageDemo` function, below, illustrates some of the logic behind montage
selection and re-referencing. 
                                                                              */
    int Show( DSI_Headset h, int n )
    {
        CHECK
        printf( "Demo %d\n", n );
        PrintSignals( h, 0, "headings" ); CHECK
        DSI_Headset_SetSampleCallback( h, PrintSignals, NULL ); CHECK
        DSI_Headset_Idle( h, 0.05 ); CHECK
        DSI_Headset_SetSampleCallback( h, NULL, NULL ); CHECK
        printf( "\n" );
        return 0;
    }
#   define SHOW( n )  if( Show( h, n ) ) return -1;

    int MontageDemo( const char * serialPort )
    {
        DSI_Headset h = DSI_Headset_New( NULL ); CHECK
        DSI_Headset_SetMessageCallback( h, Message ); CHECK
        DSI_Headset_Connect( h, serialPort ); CHECK
        DSI_Headset_StartDataAcquisition( h ); CHECK
        
        // A space-delimited string specifying the current montage (i.e. each Channel expressed
        // in terms of its Sources) can be retrieved using:

        printf( "\nDefault montage:  %s\n\n", DSI_Headset_GetMontageString( h ) ); CHECK
        
        // On initial construction, the headset looks for all the Sources that
        // it believes are connected, and creates one Channel per connected Source,
        // with the factory reference Pz as default reference. Then it calls
        // DSI_Headset_SetTraditionalReference( h, 1 ) to change the default
        // reference to linked-ears or linked-mastoids if possible.
        // Finally it adds the trigger channel and other auxiliary sensors to the
        // montage.  However, you have the power to change things as desired:
        
        DSI_Headset_ChooseChannels( h, "F3   F4  C4-P4", NULL, 0 ); SHOW( 1 )
        // This starts you out with a 3-channel montage, the first two Channels of
        // which will be referenced to the default reference (linked ears/mastoids)
        // since we have not specified that the reference should be changed.
        

        DSI_Headset_SetDefaultReference( h, "F3", 0 ); SHOW( 2 )
        // This has changed the way the first two channels are referenced, but not
        // the third (so you will effectively be looking at F3-F3, F4-F3 and C4-P4.
        // Since one of the channels is now referenced to itself, that channel will
        // always read zero.  This can be avoided by a piece of API intelligence called
        // "autoswap":


        if( DSI_Headset_SetTraditionalReference( h, 1 ) == NULL )
            DSI_Headset_SetDefaultReference( h, "FACTORY", 1 );      SHOW( 3 )
        // This sets the default reference to "A1/2+A2/2", "A1A2", "M1/2+M2/2" or "M1M2"
        // depending on what the headset has available --- if it fails, we revert to the
        // factory reference.  In other words, we're back to where we started.
        
  
        DSI_Headset_SetDefaultReference( h, "F3", 1 ); SHOW( 4 )
        // With the third argument, autoswap, now true, the previous reference is
        // swapped into any slot that becomes zero (so now you are effectively looking
        // at Pz-F3, F4-F3 and C4-P4.  The same thing could have been achieved in one
        // step by explicitly saying: 
        
        DSI_Headset_ChooseChannels( h, "Pz  F4  C4-P4",   "F3",   0 ); SHOW( 5 )

        // Channels can be queried for their Name.  By default, DSI_Channel_GetName( c ) returns the
        // name of the Source that contributes the largest positive component to Channel c.
        // However, a Channel may be renamed, for example DSI_Channel_SetName( c, "Bob" ); It is not
        // meaningful to ask a Channel for its impedance or its gain, since it may reflect multiple
        // physical sensors.


        DSI_Headset_StopDataAcquisition( h ); CHECK
        DSI_Headset_Idle( h, 1.0 ); CHECK
        
        return 0;
    }
                                                                              /*
                                                                              
6.2  EEG sensor naming conventions

A given EEG sensor may be referred to by a number of different names. For
example, the original 10-20 system gave the name "T5" to the sensor that was
renamed "P7" in later revisions (see paper defining the 10-10 system of
Sharbrough et al, 1991).

When you ask for a pointer to a `DSI_Channel` or to a `DSI_Sensor` by name,
potential "aliases" are taken into account and case is disregarded.  However,
when you ask the `DSI_Channel` or `DSI_Source` to *tell* you its name, it will
follow the headset's current naming scheme, which can be set using the function
`DSI_Headset_UseNamingScheme`. (By analogy, Robert may answer when you call him
Robert, Rob, Bob, Robbie or Bobby, but when he introduces himself, he picks his
preferred version.)  The following function illustrates this:
                                                                              */
    int NameTest( DSI_Headset h, const char * namingScheme )
    {
        DSI_Channel  ch1, ch2;
        DSI_Source src1, src2;
        
        DSI_Headset_UseNamingScheme( h, namingScheme );
        // try calling this function with "10-20" as namingScheme; then try "10-10"
        
        ch1 = DSI_Headset_GetChannelByName( h, "T5" );
        ch2 = DSI_Headset_GetChannelByName( h, "P7" );
        // pointers ch1 and ch2 should have the same value (NULL if the headset does not have this sensor)
    
        src1 = DSI_Headset_GetSourceByName( h, "T5" );
        src2 = DSI_Headset_GetSourceByName( h, "P7" );
        // pointers src1 and src2 should have the same value (NULL if the headset does not have this sensor)
        
        // The following output will depend on the chosen naming scheme:
        fprintf( stderr, "ch1:  %s\n", ( ch1  ? DSI_Channel_GetName( ch1 ) : "(null)" ) );
        fprintf( stderr, "ch2:  %s\n", ( ch2  ? DSI_Channel_GetName( ch2 ) : "(null)" ) );
        fprintf( stderr, "src1: %s\n", ( src1 ? DSI_Source_GetName( src1 ) : "(null)" ) );
        fprintf( stderr, "src2: %s\n", ( src2 ? DSI_Source_GetName( src2 ) : "(null)" ) );
        
        return 0;
    }
                                                                              /*
On initialization, a `DSI_Headset` automatically performs the following
operations to configure an initial set of known aliases. This should illustrate
the aliases that you are free to use by default, as well as the way you could
add more (but remember to check for errors if you do):

    DSI_Headset_AddSourceAliases( h, "T3 T7" );
    DSI_Headset_AddSourceAliases( h, "T4 T8" );
    DSI_Headset_AddSourceAliases( h, "T5 P7" );
    DSI_Headset_AddSourceAliases( h, "T6 P8" );
    DSI_Headset_AddSourceAliases( h, "M1 TP9" );           // left mastoid
    DSI_Headset_AddSourceAliases( h, "M2 TP10" );          // right mastoid
    DSI_Headset_AddSourceAliases( h, "M1M2 TP9TP10 LM" );  // "linked" mastoids
    DSI_Headset_AddSourceAliases( h, "A1A2 LE" );          // "linked" ears
    DSI_Headset_AddSourceAliases( h, "TRG Trig Trigger" ); // digital trigger signal
    DSI_Headset_AddSourceAliases( h, "NUL NULL ZERO" );    // always a zero-volt flat line
    DSI_Headset_AddSourceAliases( h, "CM CMF" );           // common mode follower signal


*/
