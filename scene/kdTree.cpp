#include "kdTree.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/JsonParser.h"
#include "parser/Parser.h"
#include "parser/Tokenizer.h"
#include <json.hpp>

#include "ui/TraceUI.h"
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <string.h> // for memset

#include <fstream>
#include <iostream>
#include <atomic>
#include <list>

kdTreeNodes kdTreeNodes::buildTree(std::vector<Objects*> objList, int itemsInLeaf, int depth, BoundingBox bb) {
    int depthLimit = 20; // this is wrong
    list<Objects*> leftList;
    list<Objects*> rightList;

    if(objList.size() <= itemsInLeaf || ++depth == depthLimit){
        return objList;
    }
    SplitPlane bestPlane = findBestSplitPlane(objList, bb);
    for (auto& element : objList) {
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
        return LeafNode(objList); // ??
    }
    else{
        return SplitNode(bestPlane.position, bestPlane.axis, buildTree(leftList, itemsInLeaf, depth, bestPlane.leftBoxArea), buildTree(rightList, itemsInLeaf, depth, bestPlane.rightBoxArea));
    }
}

SplitPlane kdTreeNodes::findBestSplitPlane(std::vector<Objects*> objList, BoundingBox bb) {
    SplitPlane best; //constructot
    for (int i = 0; i < 3; i++) {
        list<SplitPlane> candidates;

        // loopig thru axises
        for (auto& element : objList) {
            SplitPlane p1; // constructor
            p1.position = element.bmin[i]; // probably wrong, fix later
            p1.axis = i;
            SplitPlane p2;
            p2.position = element.bmax[i];
            p2.axis = i;
            candidates.push_back(p1);
            candidates.push_back(p2);
        }
        for (auto& element : candidates) {
            element.leftCnt = countLeftObj(); // write countLeftObj...
            element.leftBoxArea = leftBoxAreaCalc(); // write later... call boundingbox::area
            element.rightCnt = countRightObj();
            element.rightBoxArea = rightBoxAreaCalc();
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


bool SplitNode::findIntersection(ray &r, isect &i, double &tMin, double &tMax) {
    if ((position > tMin && tMin > tMax) || (tMin < tMax && tMax < position)) {
        // hitting left box only
        if (left->findIntersection(r, i, tMin, tMax)) {
            return true;
        }
    }
    else if ((position < tMin && tMin < tMax) || (tMin > tMax && tMax > position)) {
        // hitting right box only
        if (right->findIntersection(r, i, tMin, tMax)) {
            return true;
        }
    }
    else {
        // hitting both
        // is this right?
        if (left->findIntersection(r, i, tMin, tMax)) {
            return true;
        }
        if (right->findIntersection(r, i, tMin, tMax)) {
            return true;
        }
    }
    return false;
}
    

bool LeafNode::findIntersection(ray &r, isect &i, double &tMin, double &tMax) {
    for (auto& element : objList) {
        isect c_i;
        if (element->intersect(r, c_i) && c_i.getT() >= tmin && c_i.getT() <= tMax) {
            // what does this mean?
            i = c_i;
            return true; // return here or later
        }
    }
    return false;
}