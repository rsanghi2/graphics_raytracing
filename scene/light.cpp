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
  glm::dvec3 direction = -orientation;
  ray shadowRay(p + RAY_EPSILON * direction, direction, glm::dvec3(1.0, 1.0, 1.0), ray::SHADOW);
  
  isect i;
  if (scene->intersect(shadowRay, i)) {
    if (i.getMaterial().Trans()) {
      return glm::dvec3(1.0, 1.0, 1.0) * i.getMaterial().kt(i);
    }
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

  double d = glm::distance(P, position);
  double formula = 1 / (constantTerm + linearTerm * d + quadraticTerm * d * d);
  if (formula > 1) {
    formula = 1;
  }
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

  glm::dvec3 direction = glm::normalize(position - p);
  isect i;
  ray shadowRay(p + RAY_EPSILON * direction, direction, glm::dvec3(1.0, 1.0, 1.0), ray::SHADOW); 
  if (scene->intersect(shadowRay, i)) {
    double distance = glm::distance(p, position);
    if (i.getT() < distance) {
     return glm::dvec3(0, 0, 0); 
    }
  }

  return glm::dvec3(1, 1, 1);
}

#define VERBOSE 0

