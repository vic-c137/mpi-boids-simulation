#!/bin/bash
#mpirun -np 10 ./Boids nboids nloops k maxv acc width height sf1 sf2 min sf3 sf4
mpirun -np 10 ./Boids 50 500 7 10 1.25 1000 1000 1.0 32.0 50.0 8.0 10.0 > boid_data.boid