import subprocess
from os import system
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
sf3 = "8"
sf4 = "10"
dataPath = "./data/"
boidData = "boid_data.boid"
subprocess.call("mpirun -np " + np +" "+ cexe +" "+ nboids +" "+ nloops +" "+ k +" "+ maxv +" "+ acc +" "+ width +" "+ height +" "+ sf1 +" "+ sf2 +" "+ min +" "+ sf3 +" "+ sf4 + " > " + dataPath + boidData, shell=True)
jexe = "BoidModelTest"
gif = "boids.gif"
bdata = "boid_data.boid"
subprocess.call("java " + jexe + " " + gif + " " + bdata, shell=True)
system('gnuplot ./data/boid_script.gp')