#include <iostream>
#include <fstream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <string>
#include <vector>
#include <ctime>
#include "Satellite.hpp"
#define MCW MPI_COMM_WORLD
using namespace std;

const int EARTH_RADIUS = 6370; //kilometers

struct SatId {
    int rank;
    int num;
};

// generates random val from 0 to 
float randomFloat(float min, float max) {
  float range = max - min;
  float offset = range * (float)rand() / RAND_MAX;
  return min + offset;
}

// determines whether 2 points are in a threshold distance
bool collision(float point1[], float point2[], float threshold) {
  float x = point2[0] - point1[0];
  float y = point2[1] - point1[1];
  float dist = x*x + y*y;
  return dist < threshold*threshold; // square threshold instead of using sqrt (which has poor performance)
}


string coordToString(int arr[]) {
  return to_string(arr[0]) + ", " + to_string(arr[1]);
}

void swap(float &x, float &y) {
  float temp = x;
  x = y;
  y = temp;
}

void randomShift(float &perigee, float &apogee) {
  perigee += rand() % 7 - 3;
  apogee += rand() % 7 - 3;
  if (perigee < 180) perigee = 180;
  if (apogee > 2000) apogee = 2000;
  if (perigee > apogee) swap(perigee, apogee);
}

//quicksort algorithm pulled from https://www.thepolyglotdeveloper.com/2019/04/sort-vector-integers-quicksort-algorithm-cpp/
int partition(vector<int> &values, int left, int right) {
  int pivotIndex = left + (right-left) / 2;
  int pivotValue = values[pivotIndex];
  int i = left, j = right;
  int temp;
  while (i <= j) {
    while (values[i] < pivotValue) {
      i++;
    }
    while (values[j] > pivotValue) {
      j++;
    }
    if (i<= j) {
      temp = values[i];
      values[i] = values[j];
      values[j] = temp;
      i++;
      j--;
    }
  }
  return i;
}
void quicksort(vector<int> &values, int left, int right) {
  if (left<right) {
    int pivotIndex = partition(values, left, right);
    quicksort(values,left, pivotIndex-1);
    quicksort(values,pivotIndex,right);
  }
}
float angleShift() {
  float range = 6.0;
  float offset = range * (float) rand() / RAND_MAX;
  return ((-1*range/2)*offset)*(M_PI/180);
}



int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank, size;
  int data;
  int seeds[8] = {0,2,7,13,12,52,47,64};
  MPI_Status mystatus;
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);
  srand(seeds[rank]);
  int numSats = 100;      // number of satellites per processor
  int coords[3];          // coord array for sending sat positions and size info
  int cycles = 365;       // number of time positions per run of the program
  int collisionCount = 0;
  int startSize = 10;
  int splitAmt = 2;
  int splitVal = 1;

  if (rank == 0) {
    fstream fout;
    fout.open("sats.txt", ios::out);
    fout.close();
    fout.open("sats.txt", ios::app);
    fout << "{\"sats\":[\n";
    for (int i = 0; i < cycles; ++i) {
      map<string, SatId> locations; // map for detecting collision location
      map<string, bool> collisions; // map for detecting repeated collisions
      fout << "["; 
      for (int j = 1; j < size; ++j) {
        // if (i > 270) {
          // cout << "waiting on rank#" << j << " to send size for cycle#" << i << endl;
        // }
        MPI_Recv(&numSats, 1, MPI_INT, j, 0, MCW, MPI_STATUS_IGNORE);
        cout << "on cycle " << i << " rank " << j << " is sending " << numSats << " sats" << endl;
        for (int k = 0; k < numSats; ++k) {
          MPI_Recv(coords, 3, MPI_INT, j, 0, MCW, MPI_STATUS_IGNORE);
          string coordString = coordToString(coords);
          fout << "[" << coords[0] << "," << coords[1] << "," << coords[2] << "," << locations.count(coordString) << "]";
          if (j != size -1 || k != numSats-1) {
            fout << ",";            
          }
          // if collision occurred, break up satellites
          if (locations.count(coordString) != 0) {
            collisionCount++;
            // if a collision has already occurred at this location, break up both the first and second satellite (o/w second only)
            if (!collisions[coordString]) {
              SatId oldSat = locations[coordString];
              // cout << "collision between rank#" << oldSat.rank << " and rank#" << j << endl;
              MPI_Send(&oldSat.num, 1, MPI_INT, oldSat.rank, 0, MCW);
            }
            data = k;
            MPI_Send(&data, 1, MPI_INT, j, 0, MCW);
            collisions[coordString] = true;
            // best (?) option for fixing sat size: new map with fill% at each location
            // small sats fill by a small percent, large sats fill completely
            // sats have a chance to split depending on that percent
            // e.g. if 2 sats of size 10% enter a location, fill% goes to 20% and there's a 10% chance of collision
          }
          else {
            SatId si;
            si.rank = j;
            si.num = k;
            locations[coordString] = si; // add sat location to map
            collisions[coordString] = false;
          }
        }
        // if (i > 270) {
          // cout << "rank#" << j << " has finished sending for cycle#" << i << endl;
        // }
      }
      for (int sendRank = 1; sendRank < size; sendRank++) {
        data = -1;
        MPI_Send(&data, 1, MPI_INT, sendRank, 0, MCW);
      }
      fout << "]";
      if (i != cycles - 1) {
        fout << ",\n";
      }
    }
    fout << "\n]}";
    fout.close();
    cout << "There were " << collisionCount << " collisions in the year" << endl;
  }
  else {
    // create satellites
    float minOrbit = 180;
    float maxOrbit = 2000;
    vector<Satellite> sats;
    for (int i = 0; i < numSats; ++i) {
      float perigee = randomFloat(minOrbit, maxOrbit);
      float apogee = randomFloat(minOrbit, maxOrbit);
      float perigeeAngle = randomFloat(0, 2*M_PI);
      float startingAngle = randomFloat(0, 2*M_PI);
      if (perigee > apogee) {
        float tmp = apogee;
        apogee = perigee;
        perigee = tmp;
      }
      sats.push_back(Satellite(perigee+EARTH_RADIUS, apogee+EARTH_RADIUS, perigeeAngle, startingAngle, 10));
    }

    for (int i = 0; i < cycles; ++i) {
      int vecSize = sats.size();
      MPI_Send(&vecSize, 1, MPI_INT, 0, 0, MCW);
      for (int j = 0; j < sats.size(); ++j) {
        sats[j].getPosition(i, coords);
        coords[2] = sats[j].getSize();
        // tagging as i to make sure the cycle information is understood
        MPI_Send(coords, 3, MPI_INT, 0, 0, MCW); // send position back to p0 for collision detection
      }
      vector<int> satNums;
      while (true) {
        int data;
        MPI_Recv(&data, 1, MPI_INT, 0, 0, MCW, MPI_STATUS_IGNORE);
        if (data == -1) {
          break;
        }
        else {
          satNums.push_back(data);
          // reduce size of old satellite and add new satellite fragments
          int oldSize = sats[data].getSize();
          int newSize = oldSize - splitVal;
          if (newSize < 1) {
            newSize = 1;
          }
          sats[data].setSize(newSize);
          Satellite temp = sats[data];
          for (int k = 0; k < splitAmt; ++k) {
            float perigee = temp.getPerigee() - EARTH_RADIUS;
            float apogee = temp.getApogee() - EARTH_RADIUS;
            randomShift(perigee, apogee);
            sats.push_back(Satellite(perigee+EARTH_RADIUS, apogee+EARTH_RADIUS, temp.getPerigeeAngle()+angleShift(), temp.getLastTAnomaly(), newSize));
          }
        }
      }
      if (satNums.size() > 0) {
        for (int vecI = satNums.size()-1; vecI >= 0; vecI--) {
          sats.erase(sats.begin()+satNums[vecI]);
        }
      }
    }

    //using this as a guide for determing update information https://www.orbiter-forum.com/threads/so-you-want-to-calculate-orbits.24431/
  }
  MPI_Finalize();

  return 0;
}