import urllib
import numpy as np
import pylab
# url = "https://chart.googleapis.com/chart?chs=250x100&chd=t:60,40&cht=p3&chl=Hello%7CWorld"
# urllib.urlretrieve(url, 'outfile.png')

filename = "/Users/dangoodwin/Desktop/l1403491671.csv"

f = open(filename,'r')


rows = f.readlines()
print "looping through {0} lines".format(len(rows))
print rows[1]
ctr = 0
spectrogram = []
for row in rows:
    ctr = ctr+1
    print ctr
    entry = []
    if ctr>1000:
        break
    elts = row.split(',')
    if elts[0]=="timestamp":
        print "Skipping first"
        continue
    for elt in elts:

        if elt != " \n" and elt !="\n" and elt !="":
            try:
                entry.append(float(elt))
            except:
                print "issue with: ", elt
                continue


    spectrogram.append(entry)


output = np.array(spectrogram)
print "output shape ",output.shape
pylab.imshow(output)


raw_input("Press enter to close")