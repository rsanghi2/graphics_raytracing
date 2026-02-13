#pragma once

#include <iostream>
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
virtual ~kdTreeNodes(){}  ///is this rightttt???
    kdTreeNodes* buildTree(std::vector<Objects*> objList, int itemsInLeaf, int depth, BoundingBox bb, int maxDepth);
    SplitPlane<Objects> findBestSplitPlane(std::vector<Objects*> objList, BoundingBox bb);
    virtual bool findIntersection(ray &r, isect &i, double &tMin, double &tMax){
        return false;
    }
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
    //char axis[3]; // x, y, z
    int axis; // new fixed
    double position;
    kdTreeNodes<Objects>* leftChild;
    kdTreeNodes<Objects>* rightChild;
};

template <typename Objects>
class LeafNode : public kdTreeNodes<Objects> {
public:
    LeafNode (std::vector<Objects*> objs) : objList(objs) {}
    ~LeafNode() {}
    bool findIntersection(ray &r, isect &i, double &tMin, double &tMax);

private:
    std::vector<Objects*> objList;
};


template <typename Objects>
kdTreeNodes<Objects>* kdTreeNodes<Objects>::buildTree(std::vector<Objects*> objList, int itemsInLeaf, int depth, BoundingBox bb, int maxDepth) {
    //list<Objects>* leftList;
   // list<Objects>* rightList;

    if(objList.size() <= itemsInLeaf || depth >= maxDepth){
        return new LeafNode<Objects>(objList);
    }
    SplitPlane<Objects> bestPlane = findBestSplitPlane(objList, bb);
    std::vector<Objects*> leftList;
    std::vector<Objects*> rightList;
    for (auto element : objList) {
        if(element->getBoundingBox().getMin()[bestPlane.axis]<= bestPlane.position){
            //add to left list 
            leftList.push_back(element);
        }
        if(element->getBoundingBox().getMax()[bestPlane.axis]>= bestPlane.position){
            //add to right list 
            rightList.push_back(element);
        }
        // if(bestPlane.position == element->getBoundingBox().getMax()[bestPlane.axis] && bestPlane.position == element->getBoundingBox().getMin()[bestPlane.axis] && element->getN()<0/*doesnt exist currently*/)
        // {
        //     //add to left list 
        //     leftList.push_back(element);
        // }
        // else if(bestPlane.position == element->getBoundingBox().getMax()[bestPlane.axis] && bestPlane.position == element->getBoundingBox().getMin()[bestPlane.axis] && element->getN()>=0/*doesnt exist currently*/)
        // {
        //     //add to right list 
        //     rightList.push_back(element);
        // }
    }
    if(rightList.size() == 0 || leftList.size() == 0){
        return new LeafNode<Objects>(objList); // ??
    }
    else {
        kdTreeNodes<Objects>* leftTree = buildTree(leftList, itemsInLeaf, depth + 1, bestPlane.leftBBox, maxDepth); // not setting leftbbox to anythikng
        kdTreeNodes<Objects>* rightTree = buildTree(rightList, itemsInLeaf, depth + 1, bestPlane.rightBBox, maxDepth);
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
            p1.position = element->getBoundingBox().getMin()[i]; // probably wrong, fix later
            p1.min = element->getBoundingBox().getMin()[i];
            p1.max = element->getBoundingBox().getMax()[i];
            p1.axis = i;
            p1.isMin = true;

            SplitPlane<Objects> p2;
            p2.position = element->getBoundingBox().getMax()[i];
            p2.min = element->getBoundingBox().getMin()[i];
            p2.max = element->getBoundingBox().getMax()[i];
            p2.axis = i;
            p2.isMin = false;
            candidates.push_back(p1);
            candidates.push_back(p2);
        }
        list<SplitPlane<Objects>> minList;
        list<SplitPlane<Objects>> maxList; 

         for (auto& element : candidates) {
            if (element.isMin)
            {
                minList.push_back(element);
            } else {
                maxList.push_back(element);
            }
        }
        minList.sort([](const SplitPlane<Objects> a, const SplitPlane<Objects> b) {
            return a.position < b.position; // Ascending order
        });
        maxList.sort([](const SplitPlane<Objects> a, const SplitPlane<Objects> b) {
            return a.position > b.position; // descending order
        });
        for (auto& element : candidates) {
            int minCount = 0;
            int maxCount = 0;
            for(auto& minElement: minList) {
                if (minElement.position < element.position) {
                    minCount++;
                } else {
                    element.leftCnt = minCount;
                    break;
                }
            }
            for(auto& maxElement: maxList) {
                if (maxElement.position < element.position) {
                    maxCount++;
                } else {
                    element.rightCnt = maxCount;
                    break;
                }
            }
            
            // left box area calc
            double length = bb.getMax()[0] - bb.getMin()[0];
            double width = bb.getMax()[1] - bb.getMin()[1];
            double height = bb.getMax()[2] - bb.getMin()[2];
            if (element.axis == 0) {
                // divide length
                length = element.position - bb.getMin()[0];
            } 
            if (element.axis == 1) {
                // divide width
                width = element.position - bb.getMin()[1];
            } 
            if (element.axis == 2) {
                // divide height
                height = element.position - bb.getMin()[2];
            } 
            element.leftBoxArea = 2.0 * (length * width + width * height + height * length);

            element.rightBoxArea = bb.area() - element.leftBoxArea;
            element.leftBBox = bb;
            element.rightBBox = bb;
            if (element.axis == 0){
                element.leftBBox.setMax(glm::dvec3(element.position, bb.getMax()[1], bb.getMax()[2]));
                element.rightBBox.setMin(glm::dvec3(element.position, bb.getMin()[1], bb.getMin()[2]));
            } else if (element.axis == 1){
                element.leftBBox.setMax(glm::dvec3(bb.getMax()[0],element.position, bb.getMax()[2]));
                element.rightBBox.setMin(glm::dvec3( bb.getMin()[0],element.position, bb.getMin()[2]));
            } else if (element.axis == 2){
                element.leftBBox.setMax(glm::dvec3(bb.getMax()[0], bb.getMax()[1], element.position));
                element.rightBBox.setMin(glm::dvec3( bb.getMin()[0],bb.getMin()[1], element.position));
            }
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
    const glm::dvec3& origin = r.getPosition();
    const glm::dvec3& dir = r.getDirection();
   
    if (abs(dir[axis]) < RAY_EPSILON) {
        // Ray is parallel to splitting plane
        if (origin[axis] <= position) {
            return leftChild->findIntersection(r, i, tMin, tMax);
        } else {
            return rightChild->findIntersection(r, i, tMin, tMax);
        }
    }
   
    double tSplit = (position - origin[axis]) / dir[axis];
    kdTreeNodes<Objects>* first;
    kdTreeNodes<Objects>* second;
   
    if (dir[axis] > 0) {
        // + dir
        first = leftChild;  
        second = rightChild;
    } else {
        // - dir
        first = rightChild;
        second = leftChild;
    }
   
    if (tSplit > tMax || tSplit < 0) {
        // near side
        return first->findIntersection(r, i, tMin, tMax);
    } else if (tSplit < tMin) {
        // far side
        return second->findIntersection(r, i, tMin, tMax);
    } else {
        // both sides
        // near side first
        if (first->findIntersection(r, i, tMin, tSplit)) {
            return true; 
        }
        return second->findIntersection(r, i, tSplit, tMax);
    }
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

