# mpi-boids-simulation
Multi-processing swarm intelligence simulation timing and behavior analysis on MPI

Problem statement

Design and implement a complete system for anaylsis
visualization of flocking/schooling/swarming behavior
using simple agent-based modeling and simulation

Scope

a) Build and test the model for the data generation 
system (to be tested on the Beowulf cluster)

b) Build a visualization system for the data 
produced (both timing and behavior)

c) Perform sensitivity analysis on the system
to determine how parameters affect the 
timing and behavior 

Methods, Tools, Implementation

a) Model built in C LAM/MPI, tested on the cluster
1) Knn topological distance model with 3 simple rules
1.1) Attraction to precieved center of mass
1.2) Repulsion from nearby Boids
1.3) Alignment with nearby Boids
1.4) Position bounding or wrap-around
2) Client/Server communiction pattern for task handling
- Server coordinates tasks between clients
- Clients recieve updates and data tasks to handle
3) Two-phase communication protocol to reduce network communication
- Phase 1: broadcast updates for current timestep
- Phase 2: deal tasks to workers and recieve updates for next cycle

b) Visualization system has two purposes
1) Timing visualizaiton:
- Python scripts to run the program and output timing data 
in CSV format for plotting in excel
2) Behavior visualization:
- Java program to parse position/velocity (.boid) data 
into a Gnuplot script for rendering
- Gnuplot script creates GIF files for viewing behavior
- Python scripts run the system end-to-end using
subprocesses to automate data and GIF generation
- Bash scripts organize tests into sets to be run
in series

Analysis

a) Timing analysis was conducted under four varying parameters:
1) # boids
2) # loops
3) # processes
4) Knn

b) Sensitivity analysis was conducted under nine varying parameters:
1) acc
2) Knn
3) # loops
4) maxv
5) min
6) # boids
7) sf1
8) sf2
9) sf3

Visualization

a) Excel scatter plots and curve fits are used to view the 
timing data variation 

b) GIF files are used to visualize the Boid behavior	
