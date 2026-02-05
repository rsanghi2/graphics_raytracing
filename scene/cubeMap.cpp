#include "cubeMap.h"
#include "../scene/material.h"
#include "../ui/TraceUI.h"
#include "ray.h"
extern TraceUI *traceUI;

glm::dvec3 CubeMap::getColor(ray r) const {
  // YOUR CODE HERE
  // FIXME: Implement Cube Map here

  // normalizing everything just in case
  glm::dvec3 direction = glm::normalize(r.getDirection());
  double x = direction[0];
  double y = direction[1];
  double z = direction[2];

  double xAbsVal = std::abs(x); // abs?
  double yAbsVal = std::abs(y);
  double zAbsVal = std::abs(z);

  // face
  int face;
  glm::dvec2 uv; // [0] = u and [1] = v

  if (xAbsVal > yAbsVal && xAbsVal > zAbsVal) { // ==?? 
    // x face
    if (x >= 0) {
      face = 0;
      uv[0] = (y / xAbsVal + 1) / 2; // maybe normalize?????? or divide or smth
      uv[1] = (-z / xAbsVal + 1) / 2; // maybe assign absvals
    } else {
      face = 1;
      uv[0] = (y / xAbsVal + 1) / 2;
      uv[1] = (z / xAbsVal + 1) / 2;
    }
  } else if (yAbsVal > xAbsVal && yAbsVal > zAbsVal) {
    if (y >= 0) {
      face = 2;
      uv[0] = (-z / yAbsVal + 1) / 2;
      uv[1] = (x / yAbsVal + 1) / 2;
    } else {
      face = 3;
      uv[0] = (z / yAbsVal + 1) / 2;
      uv[1] = (x / yAbsVal + 1) / 2;
    }
  } else {
    if (z >= 0) {
      face = 4;
      uv[0] = (y / zAbsVal + 1) / 2;
      uv[1] = (x / zAbsVal + 1) / 2;
    } else {
      face = 5;
      uv[0] = (y / zAbsVal + 1) / 2;
      uv[1] = (-x / zAbsVal + 1) / 2;
    }
  }

  return tMap[face]->getMappedValue(uv);
  // return glm::dvec3(1,1,1);
  // return glm::dvec3(0,0,0);
}

CubeMap::CubeMap() {}

CubeMap::~CubeMap() {}

void CubeMap::setNthMap(int n, TextureMap *m) {
  if (m != tMap[n].get())
    tMap[n].reset(m);
}