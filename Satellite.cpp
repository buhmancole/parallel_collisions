#include "Satellite.hpp"
#include <cmath>
#include <cstdint>
#include <climits>
//g as gravitational constant from https://en.wikipedia.org/wiki/Gravitational_constant
// const double G = 6.674 * pow(10,-11);
//should be 10^24 instead of 10^18
// const std::uint64_t M_EARTH = 5.972 * pow(10,24);
const std::uint64_t GM = 3.983 * pow(10,14);
const int EARTH_RADIUS = 6370;


Satellite::Satellite(float perigee, float apogee, float perigeeAngle, float startingAngle, int size) {
  m_perigee = perigee;
  m_apogee = apogee;
  m_perigeeAngle = perigeeAngle;
  m_startingAngle = startingAngle;
  m_size = size;
  setSMA();
  setEccentricity();
  setMeanMotion();
  setOrbitalPeriod();

}


void Satellite::setEccentricity() {
  m_eccentricity = (m_apogee-m_perigee) / (m_apogee+m_perigee);
}


void Satellite::setSMA() {
  m_sma = (m_apogee+m_perigee)/2;
}


void Satellite::setMeanMotion() {
  m_meanMotion = sqrt(static_cast<double>(GM) / pow(m_sma,3));
}


void Satellite::setOrbitalPeriod() {
  m_orbitalPeriod = static_cast<int>((2*M_PI) * sqrt(pow(m_sma*pow(10,3),3) / (GM)));
}


double Satellite::meanAnomaly(int time) {
  return m_meanMotion * time;
}


double Satellite::eccentricAnomaly(double mAnomaly) {
  double e0 = mAnomaly;
  for (int i=0; i<100; i++) {
    e0 = e0 - ((e0-(m_eccentricity*sin(e0))-mAnomaly)/(1-(m_eccentricity * cos(e0))));
  }
  return e0;
}


double Satellite::trueAnomaly(double eAnomaly) {
  return acos((cos(eAnomaly)-m_eccentricity)/(1-(m_eccentricity*cos(eAnomaly))));
}


double Satellite::altitude(double eAnomaly) {
  return (m_sma * (1-(m_eccentricity * cos(eAnomaly)))) - EARTH_RADIUS;
}


void Satellite::getPosition(int time, int position[]) {
  //working on understanding this part
  double mAnomaly = meanAnomaly(time%m_orbitalPeriod);
  double eAnomaly = eccentricAnomaly(mAnomaly);
  double tAnomaly = trueAnomaly(eAnomaly);
  double m_lastTAnomaly = tAnomaly;
  double alt = altitude(eAnomaly);
  float x = static_cast<int>(cos(tAnomaly+m_startingAngle)*alt);
  float y = static_cast<int>(sin(tAnomaly+m_startingAngle)*alt);
  //create rotation matrix
  double transformation[2][2] = {
    {cos(m_perigeeAngle), sin(m_perigeeAngle)},
    {-sin(m_perigeeAngle), cos(m_perigeeAngle)}
  };
  //apply rotation matrix and divide by size for collision information
  position[0] = static_cast<int>(transformation[0][0]*x + transformation[1][0]*y) / m_size;
  position[1] = static_cast<int>(transformation[0][1]*x + transformation[1][1]*y) / m_size;
}







