#include "trimesh.h"
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <float.h>
#include <string.h>
#include <ostream>
#include <iostream>
#include "../ui/TraceUI.h"

extern TraceUI *traceUI;
extern TraceUI *traceUI;

using namespace std;

Trimesh::~Trimesh() {
  for (auto f : faces)
    delete f;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex(const glm::dvec3 &v) { vertices.emplace_back(v); }

void Trimesh::addNormal(const glm::dvec3 &n) { normals.emplace_back(n); }

void Trimesh::addColor(const glm::dvec3 &c) { vertColors.emplace_back(c); }

void Trimesh::addUV(const glm::dvec2 &uv) { uvCoords.emplace_back(uv); }

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace(int a, int b, int c) {
  int vcnt = vertices.size();

  if (a >= vcnt || b >= vcnt || c >= vcnt)
    return false;

  TrimeshFace *newFace = new TrimeshFace(this, a, b, c);
  if (!newFace->degen)
    faces.push_back(newFace);
  else
    delete newFace;

  // Don't add faces to the scene's object list so we can cull by bounding
  // box
  return true;
}

// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
const char *Trimesh::doubleCheck() {
  if (!vertColors.empty() && vertColors.size() != vertices.size())
    return "Bad Trimesh: Wrong number of vertex colors.";
  if (!uvCoords.empty() && uvCoords.size() != vertices.size())
    return "Bad Trimesh: Wrong number of UV coordinates.";
  if (!normals.empty() && normals.size() != vertices.size())
    return "Bad Trimesh: Wrong number of normals.";

  return 0;
}

bool Trimesh::intersectLocal(ray &r, isect &i) const {
  bool have_one = false;
  for (auto face : faces) {
    isect cur;
    if (face->intersectLocal(r, cur)) {
      if (!have_one || (cur.getT() < i.getT())) {
        i = cur;
        have_one = true;
      }
    }
  }
  if (!have_one)
    i.setT(1000.0);
  return have_one;
}

bool TrimeshFace::intersect(ray &r, isect &i) const {
  return intersectLocal(r, i);
}


// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray &r, isect &i) const {
  // YOUR CODE HERE, added stuff
  //
  // FIXME: Add ray-trimesh intersection
  

  /* To determine the color of an intersection, use the following rules:
     - If the parent mesh has non-empty `uvCoords`, barycentrically interpolate
       the UV coordinates of the three vertices of the face, then assign it to
       the intersection using i.setUVCoordinates().
     - Otherwise, if the parent mesh has non-empty `vertexColors`,
       barycentrically interpolate the colors from the three vertices of the
       face. Create a new material by copying the parent's material, set the
       diffuse color of this material to the interpolated color, and then 
       assign this material to the intersection.
     - If neither is true, assign the parent's material to the intersection.
  */
    
    bool insideOutside = false;
    glm::dvec3 a = this->parent->vertices[ids[0]]; 
    glm::dvec3 b = this->parent->vertices[ids[1]]; 
    glm::dvec3 c = this->parent->vertices[ids[2]];
    double t = glm::dot(a - r.getPosition(), this->normal) / glm::dot(this->normal, r.getDirection()); // used different formula bc we didnt have d 
    glm::dvec3 Q = r.at(t);


    glm::dvec3 vqa = Q - a;
    glm::dvec3 vqb = Q - b;
    glm::dvec3 vqc = Q - c;

      glm::dvec3 vcb = c-b;
    glm::dvec3 vac = a-c;
    glm::dvec3 vab = b-a;

    double Aa = glm::length(glm::cross(vcb, vqb)) / 2;
    double Ab = glm::length(glm::cross(vac, vqc)) / 2;
    double Ac = glm::length(glm::cross(vab, vqa)) / 2;
    double area = Aa + Ab+ Ac;
    double alpha = Aa / area;
    double beta = Ab / area;
  
    i.setT(t);
    i.setN(this->normal);
    i.setObject(this->parent);

    if (glm::dot(glm::cross(vab, vqa), this->normal)  >= 0 && 
        glm::dot(glm::cross(vcb, vqb), this->normal)  >= 0 &&
        glm::dot(glm::cross(vac, vqc), this->normal)  >= 0) {
        insideOutside = true;
    }

    if (insideOutside) {
      if (!this->parent->uvCoords.empty()) {
              // IS THIS RIGHT ???
              glm::dvec2 uv0 = this->parent->uvCoords[ids[0]]; 
              glm::dvec2 uv1 = this->parent->uvCoords[ids[1]]; 
              glm::dvec2 uv2 = this->parent->uvCoords[ids[2]];
              double gamma = 1.0 - alpha - beta; 
              glm::dvec2 interpolatedUV = alpha * uv0 + beta * uv1 + gamma * uv2; 
              i.setMaterial(this->parent->material);
              i.setUVCoordinates(interpolatedUV);


      } else if (!this->parent->vertColors.empty()) {
        glm::dvec3 interpolate = alpha * this->parent->vertColors[ids[0]] + beta * this->parent->vertColors[ids[1]] + (1 - alpha - beta) * this->parent->vertColors[ids[2]]; // is ids rightttt what is it ???
        Material material = this->parent->material;
        material.setDiffuse(interpolate);
        i.setMaterial(material);
    } else {
        i.setMaterial(this->parent->material);
      }
      return true;
    } else {
      i.setObject(this->parent);
      return false;
    }

  i.setObject(this->parent);
  return false;
}

// Once all the verts and faces are loaded, per vertex normals can be
// generated by averaging the normals of the neighboring faces.
void Trimesh::generateNormals() {
  int cnt = vertices.size();
  normals.resize(cnt);
  std::vector<int> numFaces(cnt, 0);

  for (auto face : faces) {
    glm::dvec3 faceNormal = face->getNormal();

    for (int i = 0; i < 3; ++i) {
      normals[(*face)[i]] += faceNormal;
      ++numFaces[(*face)[i]];
    }
  }

  for (int i = 0; i < cnt; ++i) {
    if (numFaces[i])
      normals[i] /= numFaces[i];
  }

  vertNorms = true;
}

