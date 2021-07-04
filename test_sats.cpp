#include <iostream>
#include "Satellite.hpp"
#include <cmath>
const int EARTH_RADIUS = 6370; //kilometers
// generates random val from min to max
float randomFloat(float min, float max) {
  float range = max - min;
  float offset = range * (float)rand() / RAND_MAX;
  return min + offset;
}
bool tooLarge(float apogee, int posX, int posY) {
  return (posX*posX) + (posY*posY) > apogee*apogee;
}
bool tooSmall(float perigee, int posX, int posY) {
  return (posX*posX) + (posY*posY) < perigee*perigee;
}
int main() {
  float minOrbit = 180;
  float maxOrbit = 2000;
  float perigee = randomFloat(minOrbit, maxOrbit);
  float apogee = randomFloat(minOrbit, maxOrbit);
  if (perigee > apogee) {
    float tmp = apogee;
    apogee = perigee;
    perigee = tmp;
  }
  float perigeeAngle = randomFloat(0, 2*M_PI);
  float startingAngle = randomFloat(0, 2*M_PI);
  std::cout << "perigee " << perigee << std::endl;
  std::cout << "apogee " << apogee << std::endl;
  std::cout << "perigee angle " << perigeeAngle << std::endl;
  std::cout << "starting angle " << startingAngle << std::endl;
  Satellite sat = Satellite(perigee+EARTH_RADIUS, apogee+EARTH_RADIUS, perigeeAngle, startingAngle, 10);
  int secsInDay = 60*60*24;
  int position[2];
  for (int i=0; i<365; i++) {
    sat.getPosition(i*secsInDay, position);
    std::cout << "Position on day " << i << " is " << position[0] << "," << position[1] << std::endl;
    // if (tooLarge(apogee,position[0],position[1]) || tooSmall(perigee, position[0], position[1])) {
      // std::cout << "error at day " << i << std::endl;
    // }
  }
  return 0;
}
