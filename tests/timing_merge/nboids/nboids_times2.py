# Import statements
import subprocess
from os import system
# Variable declarations
np = "10"
cexe = "./Times"
nboids = "50"
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
timeData = "time_data_nboids2.csv"
# Test calls
collection = [1, 2, 3, 5, 10, 25, 30, 35, 40, 45, 50]
for i in collection:
	print "Running test %s" % (str(i)) 
	nboids = str(i)
	subprocess.call("mpirun -np " + np +" "+ cexe +" "+ nboids +" "+ nloops +" "+ k +" "+ maxv +" "+ acc +" "+ width +" "+ height +" "+ sf1 +" "+ sf2 +" "+ min +" "+ sf3 +" "+ sf4 + " >> " + dataPath + timeData, shell=True)