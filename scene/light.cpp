#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3 &) const {
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}

glm::dvec3 DirectionalLight::shadowAttenuation(const ray &r,
                                               const glm::dvec3 &p) const {
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.

  ray reverse;
  isect i;
  glm::dvec3 revOrien;
  revOrien[0] = -orientation[0];
  revOrien[1] = -orientation[1];
  revOrien[2] = -orientation[2]; // ????? think its rihgt but hello
  reverse.setDirection(revOrien);
  reverse.setPosition(p);
  if (scene->intersect(reverse, i)) {
    return glm::dvec3(0, 0, 0);
  }
  return glm::dvec3(1.0, 1.0, 1.0);
}

glm::dvec3 DirectionalLight::getColor() const { return color; }

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3 &) const {
  return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3 &P) const {
  // YOUR CODE HERE


  // You'll need to modify this method to attenuate the intensity
  // of the light based on the distance between the source and the
  // point P.  For now, we assume no attenuation and just return 1.0
  double d = sqrt(((P[0] - position[0]) * (P[0] - position[0])) + ((P[1] - position[1]) * (P[1] - position[1])) + ((P[2] - position[2]) * (P[2] - position[2])));
  double formula = 1 / (constantTerm + linearTerm * d + quadraticTerm * d * d);
  return formula;
}

glm::dvec3 PointLight::getColor() const { return color; }

glm::dvec3 PointLight::getDirection(const glm::dvec3 &P) const {
  return glm::normalize(position - P);
}

glm::dvec3 PointLight::shadowAttenuation(const ray &r,
                                         const glm::dvec3 &p) const {
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.

  double direction = glm::normalize(position - p); // TODO check p type
  isect i;
  scene.intersect(r, i);
  double t = i.getT(); 
  double distance = sqrt(((p[0] - position[0]) * (p[0] - position[0])) + ((p[1] - position[1]) * (p[1] - position[1])) + ((p[2] - position[2]) * (p[2] - position[2])));

  if (t < distance) {
    return glm::dvec3(0, 0, 0); 
  }

  return glm::dvec3(1, 1, 1);
}

#define VERBOSE 0

