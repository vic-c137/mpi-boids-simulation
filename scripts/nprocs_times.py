# Import statements
import subprocess
from os import system
# Variable declarations
np = "5"
cexe = "./Times"
nboids = "500"
nloops = "1000"
k = "7"
maxv = "10"
acc = "1.25"
width = "1000"
height = "1000"
sf1 = "1"
sf2 = "32"
min = "50"
sf3 = "8"
sf4 = "10"
dataPath = "./data/"
timeData = "time_data_nprocs1.csv"
# Test calls
collection = [2, 3, 4, 5]
for i in collection:
	print "Running test %s" % (str(i)) 
	np = str(i)
	subprocess.call("lamhalt", shell=True)
	subprocess.call("lamboot -v ./bhosts.cpu1", shell=True)
	subprocess.call("mpirun -np " + np +" "+ cexe +" "+ nboids +" "+ nloops +" "+ k +" "+ maxv +" "+ acc +" "+ width +" "+ height +" "+ sf1 +" "+ sf2 +" "+ min +" "+ sf3 +" "+ sf4 + " >> " + dataPath + timeData, shell=True)