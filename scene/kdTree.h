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
class SplitPlane {
public:
    double position = 0.0;
    double min = 0.0;
    double max = 0.0;
    int leftCnt = 0;
    int rightCnt = 0;
    double leftBoxArea = 0.0;
    double rightBoxArea = 0.0;
    int axis = 0; // 0 = x, ...
    BoundingBox leftBBox;
    BoundingBox rightBBox;
    bool isMin;
};

template <typename Objects>
class kdTreeNodes {
public:
    kdTreeNodes* buildTree(std::vector<Objects*> objList, int itemsInLeaf, int depth, BoundingBox bb);
    SplitPlane<Objects> findBestSplitPlane(std::vector<Objects*> objList, BoundingBox bb);
    bool findIntersection(ray &r, isect &i, double &tMin, double &tMax);
};

template <typename Objects>
class SplitNode : public kdTreeNodes<Objects> {
public:
    SplitNode(double pos, int ax, kdTreeNodes<Objects>* l, kdTreeNodes<Objects>* r) : position(pos), axis(ax), leftChild(l), rightChild(r) {}

    ~SplitNode () {
        delete leftChild; 
        delete rightChild;
    } //??
    bool findIntersection(ray &r, isect &i, double &tMin, double &tMax);
    
private:
    // plane will be represented by dvec2 w [0] = axis, [1] = pos
    char axis[3]; // x, y, z
    double position;
    kdTreeNodes<Objects>* leftChild;
    kdTreeNodes<Objects>* rightChild;
};

template <typename Objects>
class LeafNode : public kdTreeNodes<Objects> {
public:
LeafNode (std::vector<Objects*> objs) : objList(objs) {}
    bool findIntersection(ray &r, isect &i, double &tMin, double &tMax);
private:
    std::vector<Objects*> objList;
};


template <typename Objects>
kdTreeNodes<Objects>* kdTreeNodes<Objects>::buildTree(std::vector<Objects*> objList, int itemsInLeaf, int depth, BoundingBox bb) {
    int depthLimit = 20; // this is wrong
    list<Objects>* leftList;
    list<Objects>* rightList;

    if(objList.size() <= itemsInLeaf || ++depth == depthLimit){
        return new LeafNode<Objects>(objList);
    }
    SplitPlane<Objects> bestPlane = findBestSplitPlane(objList, bb);
    // std::vector<Objects*> leftList;
    // std::vector<Objects*> rightList;
    for (auto element : objList) {
        if(element.bmin[bestPlane.axis]< bestPlane.position){
            //add to left list 
            leftList.push_back(element);
        }
        if(element.bmin[bestPlane.axis]> bestPlane.position){
            //add to right list 
            rightList.push_back(element);
        }
        if(bestPlane.position == element.bmax[bestPlane.axis] && bestPlane.position == element.bmin[bestPlane.axis] && element.getN()<0/*doesnt exist currently*/)
        {
            //add to left list 
            leftList.push_back(element);
        }
        else if(bestPlane.position == element.bmax[bestPlane.axis] && bestPlane.position == element.bmin[bestPlane.axis] && element.getN()>=0/*doesnt exist currently*/)
        {
            //add to right list 
            rightList.push_back(element);
        }
    }
    if(rightList.size() == 0 || leftList.size() == 0){
        return new LeafNode<Objects>(objList); // ??
    }
    else {
        kdTreeNodes<Objects>* leftTree = buildTree(leftList, itemsInLeaf, depth + 1, bestPlane.leftBBox); // not setting leftbbox to anythikng
        kdTreeNodes<Objects>* rightTree = buildTree(rightList, itemsInLeaf, depth + 1, bestPlane.rightBBox);
        return new SplitNode(bestPlane.position, bestPlane.axis, leftTree, rightTree);
    }
}

template <typename Objects>
SplitPlane<Objects> kdTreeNodes<Objects>::findBestSplitPlane(std::vector<Objects*> objList, BoundingBox bb) {
    SplitPlane<Objects> best; //constructot
            //new list

    for (int i = 0; i < 3; i++) {
        list<SplitPlane<Objects>> candidates;

        // looping thru axises
        for (auto element : objList) {
            SplitPlane<Objects> p1; // constructor
            p1.position = element.bmin[i]; // probably wrong, fix later
            p1.min = element.bmin[i];
            p1.max = element.bmax[i];
            p1.axis = i;
            p1.min = true;

            SplitPlane<Objects> p2;
            p2.position = element.bmax[i];
            p2.min = element.bmin[i];
            p2.max = element.bmax[i];
            p2.axis = i;
            p2.min = false;
            candidates.push_back(p1);
            candidates.push_back(p2);
        }
        list<SplitPlane<Objects>> minList;
        list<SplitPlane<Objects>> maxList; 

         for (auto& element : candidates) {
            if (element.min)
            {
                minList.push_back(element);
            }
            else{
                maxList.push_back(element);
            }
        }
        minList.sort(sortMin()); //is this right
        maxList.sort(sortMax()); //is this right
        for (auto& element : candidates) {
            int minCount = 0;
            int maxCount = 0;
            for(auto& minElement: minList) {
                if(minElement.min < element.min) {
                    minCount++;
                }
                else {
                    element.leftCnt = minCount;
                    break;
                }
                 
            }
            for(auto& maxElement: maxList) {
                if(maxElement.max < element.max) {
                    maxCount++;
                }
                else {
                    element.rightCnt = maxCount;
                }
            }
            
            // left box area calc
            double length = bmax[0] - bmin[0];
            double width = bmax[1] - bmin[1];
            double height = bmax[2] - bmin[2];
            if (element.axis == 0) {
                // divide length
                length = position - bmin[0];
            } 
            if (element.axis == 1) {
                // divide width
                length = position - bmin[1];
            } 
            if (element.axis == 2) {
                // divide height
                length = position - bmin[2];
            } 
            element.leftBoxArea = 2.0 * (length * width + width * height + height * length);

            element.rightBoxArea = bb.area() - element.leftBoxArea;
        }

        double minSAM = 999999.0; // prob better way to do this
        for (auto& element : candidates) {
            double SAM = (element.leftCnt * element.leftBoxArea + element.rightCnt * element.rightBoxArea) / bb.area();
            if (SAM < minSAM) {
                minSAM = SAM;
                best = element;
            }
        }
    }
    return best;
}

template <typename Objects>
bool SplitNode<Objects>::findIntersection(ray &r, isect &i, double &tMin, double &tMax) {
    if ((position > tMin && tMin > tMax) || (tMin < tMax && tMax < position)) {
        // hitting left box only
        if (leftChild->findIntersection(r, i, tMin, tMax)) {
            return true;
        }
    }
    else if ((position < tMin && tMin < tMax) || (tMin > tMax && tMax > position)) {
        // hitting right box only
        if (rightChild->findIntersection(r, i, tMin, tMax)) {
            return true;
        }
    }
    else {
        // hitting both
        // is this right?
        if (leftChild->findIntersection(r, i, tMin, tMax)) {
            return true;
        }
        if (rightChild->findIntersection(r, i, tMin, tMax)) {
            return true;
        }
    }
    return false;
}
    
template <typename Objects>
bool LeafNode<Objects>::findIntersection(ray &r, isect &i, double &tMin, double &tMax) {
    bool hit = false;
    for (auto element : objList) {
        isect c_i;
        if (element->intersect(r, c_i) && c_i.getT() >= tMin && c_i.getT() <= tMax) {
            // what does this mean?
            i = c_i;
            tMax = c_i.getT(); //?
            hit = true; // return here or later
        }
    }
    return hit;
}

bool sortMin(const SplitPlane<Objects>a, const SplitPlane<Objects> b) {
        return a.position < b.position; // Ascending order
    };

bool sortMax(const SplitPlane<Objects> a, const SplitPlane<Objects> b) {
        return a.position > b.position; // descending order
    };