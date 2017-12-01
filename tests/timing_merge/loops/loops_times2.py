# Import statements
import subprocess
from os import system
# Variable declarations
np = "5"
cexe = "./Times"
nboids = "500"
nloops = "500"
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
timeData = "time_data_loops2.csv"
# Test calls
collection = [1, 5, 10, 50, 100, 500, 1000, 5000]
for i in collection:
	print "Running test %s" % (str(i)) 
	nloops = str(i)
	subprocess.call("mpirun -np " + np +" "+ cexe +" "+ nboids +" "+ nloops +" "+ k +" "+ maxv +" "+ acc +" "+ width +" "+ height +" "+ sf1 +" "+ sf2 +" "+ min +" "+ sf3 +" "+ sf4 + " >> " + dataPath + timeData, shell=True)