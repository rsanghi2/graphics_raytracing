#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI *traceUI;

#include "../fileio/images.h"
#include <glm/gtx/io.hpp>
#include <iostream>

using namespace std;
extern bool debugMode;

Material::~Material() {}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene *scene, const ray &r, const isect &i) const {
  // YOUR CODE HERE

  // For now, this method just returns the diffuse color of the object.
  // This gives a single matte color for every distinct surface in the
  // scene, and that's it.  Simple, but enough to get you started.
  // (It's also inconsistent with the phong model...)

  // Your mission is to fill in this method with the rest of the phong
  // shading model, including the contributions of all the light sources.
  // You will need to call both distanceAttenuation() and
  // shadowAttenuation()
  // somewhere in your code in order to compute shadows and light falloff.
  //  if( debugMode )
  //    std::cout << "Debugging Phong code..." << std::endl;

  // When you're iterating through the lights,
  // you'll want to use code that looks something
  // like this:
  //


  glm::dvec3 intensity = i.getMaterial().ke(i) +
                         i.getMaterial().ka(i) * scene->ambient();
 
  glm::dvec3 intersection = r.at(i.getT());
 
  const glm::dvec3 eye = scene->getCamera().getEye();
  glm::dvec3 view = glm::normalize(eye - intersection);

  for (const auto& pLight : scene->getAllLights())
  {
    // attenuation
    double distAtten = pLight->distanceAttenuation(intersection);
    glm::dvec3 shadowAtten = pLight->shadowAttenuation(r, intersection);
    glm::dvec3  atten = distAtten * shadowAtten;

    // diffuse
    glm::dvec3 L = pLight->getDirection(intersection);
    glm::dvec3 diffuseTerm = glm::dvec3(0, 0, 0);
    double nDotL = glm::dot(i.getN(), L);
    if (nDotL > 0) {
      diffuseTerm = i.getMaterial().kd(i) * pLight->getColor() * nDotL;
    }
   
    // specular
    glm::dvec3 specTerm = glm::dvec3(0, 0, 0);
    glm::dvec3 R = glm::reflect(-L, i.getN());
    double rDotV = glm::dot(R, view);
    if (rDotV > 0) {
      double pow = std::pow(rDotV, i.getMaterial().shininess(i)); // need std?
      specTerm = i.getMaterial().ks(i) * pLight->getColor() * pow;
    }

    intensity = intensity + atten * (diffuseTerm + specTerm);
  }
  return intensity;
}

TextureMap::TextureMap(string filename) {
  data = readImage(filename.c_str(), width, height);
  if (data.empty()) {
    width = 0;
    height = 0;
    string error("Unable to load texture map '");
    error.append(filename);
    error.append("'.");
    throw TextureMapException(error);
  }
}

glm::dvec3 TextureMap::getMappedValue(const glm::dvec2 &coord) const {
  // YOUR CODE HERE
  //
  // In order to add texture mapping support to the
  // raytracer, you need to implement this function.
  // What this function should do is convert from
  // parametric space which is the unit square
  // [0, 1] x [0, 1] in 2-space to bitmap coordinates,
  // and use these to perform bilinear interpolation
  // of the values.

  return glm::dvec3(1, 1, 1);
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const {
  // YOUR CODE HERE
  //
  // In order to add texture mapping support to the
  // raytracer, you need to implement this function.
  return glm::dvec3(1, 1, 1);
}

glm::dvec3 MaterialParameter::value(const isect &is) const {
  if (0 != _textureMap)
    return _textureMap->getMappedValue(is.getUVCoordinates());
  else
    return _value;
}

double MaterialParameter::intensityValue(const isect &is) const {
  if (0 != _textureMap) {
    glm::dvec3 value(_textureMap->getMappedValue(is.getUVCoordinates()));
    return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
  } else
    return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}