MPI is required to run this program, but satellites.txt alreaedy contains the output from a sample run.

The purpose of this project is to simulate satellites flying around the earth and colliding with each other. When the simulated satellites collide, they split into two pieces with a slightly adjusted angle, perigee, and apogee. We used a minimum perigee distance of 180 km and a maximum apogee distance of 2000 km to simulate low Earth orbit.

Compile the program with "mpic++ collisions.cpp Satellite.cpp".
Run the program with "mpirun -np 4 a.out".
Running with more than 4 processes largely increases the runtime of the program.

After satellite positions have been generated, the data can be viewed by opening index.html in a browser and using the file picker to select the sats.txt file.
