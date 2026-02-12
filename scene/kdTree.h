#pragma once

#include <list>
#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "bbox.h"
#include "camera.h"
#include "material.h"
#include "ray.h"

#include <glm/geometric.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

using namespace std;

// Note: you can put kd-tree here
template <typename Objects>
class kdTreeNodes {
public:
    kdTreeNodes buildTree(std::vector<Objects*> objList, int itemsInLeaf, int depth, BoundingBox bb);
    glm::dvec2 findBestSplitPlane(std::vector<Objects*> objList, BoundingBox bb);
    bool findIntersection(ray &r, isect &i, double &tMin, double &tMax);
};
template <typename Objects>
class SplitNode : public kdTreeNodes<Objects> {
public:
    bool findIntersection(ray &r, isect &i, double &tMin, double &tMax);
    
private:
    // plane will be represented by dvec2 w [0] = axis, [1] = pos
    char axis[3]; // x, y, z
    float position;
    kdTreeNodes<Objects*> *left;
    kdTreeNodes<Objects*> *right;
};
template <typename Objects>
class LeafNode : public kdTreeNodes<Objects> {
public:
    bool findIntersection(ray &r, isect &i, double &tMin, double &tMax);
private:
    std::vector<Objects*> objList;
};