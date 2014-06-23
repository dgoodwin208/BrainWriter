import pylab as plt
import numpy as np

import scipy
import numpy as np
from sklearn import svm
import itertools
from sklearn import preprocessing
from scipy.signal import butter, lfilter


#Global Parameters
NBINS_PER_FFT = 10 #for deciding the feature vector length
#data_file = "/Users/dangoodwin/Code/OpenFrameworks/apps/Brainwriter/Data060914/firstmentalmath(post1drink).csv"
data_file = "/Users/dangoodwin/Desktop/thinkingandblinking.csv"
INPUT_CHANNELS = [1,3]
OUTPUT_CHANNELS = [3]
VIZ_ONLY = True
###Create the spectrogram:
framesz = 1.0 # with a frame size of 50 milliseconds
hop = 1.0     # how far to go before defining the next frame
Fs = 250.0
lowcut_freq = .5
highcut_freq = 70.


#Spectrogram code used from: http://stackoverflow.com/questions/2459295/stft-and-istft-in-python
def stft(x, fs, framesz, hop, demean=False):

    if demean:
        x = x - np.mean(x)
    framesamp = int(framesz*fs)
    hopsamp = int(hop*fs)
    w = scipy.hamming(framesamp)
    X = np.zeros([int(fs/2),len(range(0, len(x)-framesamp, hopsamp) )])
    row_ctr = 0
    for i in range(0, len(x)-framesamp, hopsamp):
        weighted = w*x[i:i+framesamp]
        ps = np.abs(np.fft.fft(weighted))**2
        freqs = np.fft.fftfreq(w.size, 1/Fs)
        idxs = freqs>=0
        col = np.transpose(-10*np.log(ps[idxs]))
        X[:,row_ctr] = col
        row_ctr+=1
    return X


#Take the freq magnitude vectors per channel (for a specific time slice)
#bin them into NBINS_PER_FFT, then reshape them into a 1D vector
#x is is a matrix of [freq mags, chan]
def make_input_vector(x):

    #How many freqs are we binning into one slot?
    bin_size = x.shape[0]/NBINS_PER_FFT

    numchans = x.shape[1]
    print numchans

    #Putting each channels result in a row of a matrix
    x_bin = np.zeros([numchans,NBINS_PER_FFT])

    for j in range(0,numchans):
        for i in range(0,NBINS_PER_FFT):
            x_bin[j,i] = np.sum(x[i*bin_size:(i+1)*bin_size,j])

    #results = np.concatenate([x1_bin, x2_bin])

    return x_bin.reshape(NBINS_PER_FFT*numchans)


def butter_bandpass(lowcut, highcut, fs, order=5):
    nyq = 0.5 * fs
    low = lowcut / nyq
    high = highcut / nyq
    b, a = butter(order, [low, high], btype='band')
    return b, a

def butter_bandpass_filter(data, lowcut, highcut, fs, order=5):
    b, a = butter_bandpass(lowcut, highcut, fs, order=order)
    y = lfilter(b, a, data)
    return y

def readDataFile(data_file, input_channel_indices, outputchannel_idx,xchannel=-1):
    f = open(data_file,'r')
    #Note that there was a glitch that switched the 'prompt' and 'timestamp' headers
    lines = f.readlines()
    inputsignals = []
    labelsignal = []

    x = []
    for line in lines:
        #If it's the first line (headers,etc) ignore
        if line[0]=="prompt" or line[0]=="":
            continue

        elts = line.split(',')
        if len(elts)==1:
            continue
        if elts[0]=='timestamp':
            continue

        row = []
        for in_idx in input_channel_indices:
            row.append(float(elts[in_idx]))

        inputsignals.append(row)

        if xchannel>=0:
            x.append(int(elts[xchannel]))

        if outputchannel_idx:
            for chan in outputchannel_idx:
                labelsignal.append(float(elts[chan])) #The "Fire!" signal is appended at the bottom

        f.close()

    count = len(inputsignals)
    input_data = np.zeros([count, len(input_channel_indices)])
    for i in range(0,count):
        for j in range(0,len(input_channel_indices)):
            input_data[i,j] = inputsignals[i][j]

    labelsignal = np.array(labelsignal)
    #input_data = np.zeros([len(input_data[0]), len(input_channel_indices)])
    #for chan in range(0,len(input_channel_indices)):
    #    input_data[:,chan] =

    return input_data, labelsignal, x

def remove_artifacts(x):
    mean = np.mean(x)
    std = np.std(x)

    for i in range(1, len(x)):
        if abs(x[i])>mean+3*std:
            x[i] = x[i-1]
    return x

#Loops over the output signal and take the mode of the y at each time window
def block_output(y, fs, framesz, hop):
    framesamp = int(framesz*fs)
    hopsamp = int(hop*fs)
    w = scipy.hamming(framesamp)
    Y = np.zeros(len(range(0, len(y)-framesamp, hopsamp)) )
    idx = 0
    for i in range(0, len(y)-framesamp, hopsamp):
        #What are the 1's and 0's at this time?
        if i+framesamp<len(y):
            vals = y[i:i+framesamp]
        else:
            vals = y[i:]
        #Mode returns [val, frequency]
        mode = scipy.stats.mode(vals)[0]
        if mode==0:
            mode=-1.0
        Y[idx] = mode
        idx+=1
    return Y


def main():


    (inputdata, labelsignal, x) = readDataFile(data_file, INPUT_CHANNELS, OUTPUT_CHANNELS, xchannel=0)

    # print len(x)
    # plt.plot(x,'.')
    # for i in range(1,len(x)):
    #     if x[i]-x[i-1]>1:
    #         print "issue at ",i

    #convert the lists into numpy arrays
    num_channels = len(INPUT_CHANNELS)
    inputdata = inputdata*.02235 #scale it to microvolts



    #firesignal = np.array(outputdata)

    #remove the artifacts
    # signal1 = remove_artifacts(signal1)
    # signal2 = remove_artifacts(signal2)
    # print signal1

    #bandpass the data
    # for chan in range(0,num_channels):
    #     inputdata[:,chan]= butter_bandpass_filter(inputdata[:,chan], lowcut_freq, highcut_freq, Fs, order=4)

    alpha = butter_bandpass_filter(inputdata[:,0], 10, 20, Fs, order=6)
    beta = butter_bandpass_filter(inputdata[:,0], 22, 35, Fs, order=6)


    f = open("samplesigs.csv",'w')
    for x in range(0,500):
        f.write("{0},{1}\n".format(alpha[16000+x],beta[16000+x]))
    f.close()
    print "Written!"
    plt.figure
    plt.plot(alpha)
    plt.plot(beta)
    #or just demean the data
    # signal1 = signal1 - float(np.mean(signal1))
    # signal2 = signal2 - float(np.mean(signal2))



    #truncate the data to deal with the edge effects of the bandpass filter
    # STARTIDX=1 #1 means no filtering
    # inputdata = inputdata[STARTIDX:,:]
    # labelsignal = labelsignal[STARTIDX:]

    #The two channels worth of signal are turned into spectrograms
    X0_temp = stft(inputdata[:,0], Fs, framesz, hop, demean=True)
    # [number of frequencies] x [number of time slices ]  x [number of channels]

    #X will hold the spectrogram data.
    #Dimensions are [frequency mag, time , channel]
    X = np.zeros([X0_temp.shape[0], X0_temp.shape[1], inputdata.shape[1]])

    for chan in range(0,num_channels):
        tempX = stft(inputdata[:,chan], Fs, framesz, hop, demean=True)
        X[:,:,chan] = scipy.absolute(tempX)

    print X.shape
    #X_spectrogram = scipy.absolute(X1)

    #X2 = stft(signal2, Fs, framesz, hop)
    #X2_spectrogram = scipy.absolute(X2)

    #The output signal is extracted to math the inputs
    y = block_output(labelsignal, Fs, framesz, hop)

    fig = plt.figure(num=None, figsize=(16,8), dpi=80, facecolor='w', edgecolor='k')
    for chan in range(0,inputdata.shape[1]):
        plt.plot(inputdata[:,chan])
    plt.ylabel('uV')
    plt.xlabel('time index')

    for chan in range(0,num_channels):
        fig = plt.figure(num=None, figsize=(16,8), dpi=80, facecolor='w', edgecolor='k')
        plt.imshow(X[:,:,chan], origin='lower', aspect='auto',interpolation='nearest')
        plt.xlabel('Time')
        plt.ylabel('Frequency')
        plt.title('Channel {0}'.format(INPUT_CHANNELS[chan]))

    if VIZ_ONLY:
        raw_input("Press <RETURN> to close")
        return

    #Shrink each timeslice of the two spectrograms into a histogram format that can be used in
    #The input rows for each timeslice
    sample_input_vector = make_input_vector(X[:, 0, :])
    x = np.zeros([X.shape[1],len(sample_input_vector)]) #This is the final input signal into the FFT

    for i in range(X.shape[1]): #For each timeslice
        x[i,:] = make_input_vector(X[:, i, :])


    print "Length of input vector: " , X.shape[1]
    #NOW we have data ready for SVM!
    #Do a leave-one-out analysis to see how it performs
    number_corrects = 0
    confusion_matrix = np.zeros([2,2])

    #fig = plt.figure(num=None, figsize=(16,8), dpi=80, facecolor='w', edgecolor='k')
    #plt.imshow(x)
    #print x.shape
    for loo_idx in range(0,len(x)):
        #Create the test and train data

        y_test = y[loo_idx]
        x_test = x[loo_idx,:]

        y_train = np.array(list(itertools.compress(y, [i != loo_idx for i in range(len(y))])))
        x_train = np.delete(x,loo_idx,axis=0)

        #Debugging
        if np.isinf(x_train).any():
            print "xtrain", loo_idx
            break

        #Preprocess using the awesome preprocessing pipeline
        #documentation here: http://scikit-learn.org/stable/modules/preprocessing.html#preprocessing
        scaler = preprocessing.StandardScaler().fit(x_train)
        x_train_processed = scaler.transform(x_train)

        clf = svm.SVC(C=10,class_weight='auto')
        #Trying to do a L1 regularization
        #clf = svm.LinearSVC(C=1,class_weight='auto', loss='l2', penalty='l1', dual=False)
        clf.fit(x_train_processed, y_train)

        x_test_processed= scaler.transform(x_test)

        guess = clf.predict(x_test_processed)

        #print y_test, guess

        #Populate the confusion matrix
        guess_idx = int(guess[0])
        if guess_idx == -1:
            guess_idx = 0
        actual_idx = y_test
        if actual_idx == -1:
            actual_idx = 0
        confusion_matrix[int(actual_idx), int(guess_idx)] +=1

        if int(y_test) == int(guess[0]):
            number_corrects +=1

        #break

    print "Total accuracy: {0}/{1} = {2}".format(number_corrects, len(x), float(number_corrects)/len(x))
    print confusion_matrix

    fig = plt.figure(num=None, figsize=(16,8), dpi=80, facecolor='w', edgecolor='k')
    plt.plot(X[:,4,0])
    plt.title("Time Slice FFT")
    raw_input("Press <RETURN> to close")
if __name__ == "__main__":
    main()
