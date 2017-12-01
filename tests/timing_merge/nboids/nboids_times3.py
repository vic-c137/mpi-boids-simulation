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
timeData = "time_data_nboids3.csv"
# Test calls
collection = [100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2100, 2200, 2300, 2400, 2500]
for i in collection:
	print "Running test %s" % (str(i)) 
	nboids = str(i)
	subprocess.call("mpirun -np " + np +" "+ cexe +" "+ nboids +" "+ nloops +" "+ k +" "+ maxv +" "+ acc +" "+ width +" "+ height +" "+ sf1 +" "+ sf2 +" "+ min +" "+ sf3 +" "+ sf4 + " >> " + dataPath + timeData, shell=True)