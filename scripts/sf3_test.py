# Import statements
import subprocess
from os import system
# Variable declarations
np = "10"
cexe = "./Boids"
nboids = "50"
nloops = "500"
k = "7"
maxv = "10"
acc = "1.25"
width = "1000"
height = "1000"
sf1 = "1"
sf2 = "32"
min = "50"
#sf3 = "8"
sf4 = "10"
dataPath = "./data/"
jexe = "BoidModelTest"
# Test calls
collection = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
for i in collection:
	print "Running test %s" % (str(i)) 
	boidData = "run"+str(i)+".boid"
	gif = "run"+str(i)+".gif"
	sf3 = str(i)
	subprocess.call("mpirun -np " + np +" "+ cexe +" "+ nboids +" "+ nloops +" "+ k +" "+ maxv +" "+ acc +" "+ width +" "+ height +" "+ sf1 +" "+ sf2 +" "+ min +" "+ sf3 +" "+ sf4 + " > " + dataPath + boidData, shell=True)
	subprocess.call("java " + jexe + " " + gif + " " + boidData, shell=True)
	system('gnuplot ./data/boid_script.gp')