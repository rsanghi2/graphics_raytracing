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
 	virtual ~kdTreeNodes(){}
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
 	}
 	bool findIntersection(ray &r, isect &i, double &tMin, double &tMax);

private:
 	int axis;
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
 	// Base case: create leaf node
 	if(objList.size() <= itemsInLeaf || depth >= maxDepth){
 		return new LeafNode<Objects>(objList);
 	}

 	// Find best split plane
 	SplitPlane<Objects> bestPlane = findBestSplitPlane(objList, bb);

 	// Partition objects into left and right lists
 	std::vector<Objects*> leftList;
 	std::vector<Objects*> rightList;

 	for (auto element : objList) {
 		BoundingBox objBB = element->getBoundingBox();
 		double objMin = objBB.getMin()[bestPlane.axis];
 		double objMax = objBB.getMax()[bestPlane.axis];

 		// Object overlaps left side if its min is less than split position
 		bool inLeft = (objMin < bestPlane.position);
 		// Object overlaps right side if its max is greater than split position 
 		bool inRight = (objMax > bestPlane.position);

 		// Handle degenerate case: object lies exactly on the plane
 		if (objMin == bestPlane.position && objMax == bestPlane.position) {
 			// Put it on the left side only
    	leftList.push_back(element);
 		} else {
 			if (inLeft) {
    		leftList.push_back(element);
 			}
 			if (inRight) {
    		rightList.push_back(element);
 			}
 		}
 	}	

 	// If split didn't help, create leaf node
 	if(rightList.size() == 0 || leftList.size() == 0){
 		return new LeafNode<Objects>(objList);
 	}

 	// Recursively build left and right subtrees
 	kdTreeNodes<Objects>* leftTree = buildTree(leftList, itemsInLeaf, depth + 1, bestPlane.leftBBox, maxDepth);
 	kdTreeNodes<Objects>* rightTree = buildTree(rightList, itemsInLeaf, depth + 1, bestPlane.rightBBox, maxDepth);

 	return new SplitNode<Objects>(bestPlane.position, bestPlane.axis, leftTree, rightTree);
}

template <typename Objects>
SplitPlane<Objects> kdTreeNodes<Objects>::findBestSplitPlane(std::vector<Objects*> objList, BoundingBox bb) {
 	SplitPlane<Objects> best;
 	double minCost = 1e30; // Use a very large number

 	// Try each axis
 	for (int axis = 0; axis < 3; axis++) {
 		std::list<SplitPlane<Objects>> candidatesList;

 		// Create candidate split planes at object boundaries
 		for (auto element : objList) {
 			BoundingBox objBB = element->getBoundingBox();

 			SplitPlane<Objects> p1;
 			p1.position = objBB.getMin()[axis];
 			p1.axis = axis;
 			p1.isMin = true;

 			SplitPlane<Objects> p2;
 			p2.position = objBB.getMax()[axis];
 			p2.axis = axis;
 			p2.isMin = false;

 			candidatesList.push_back(p1);
 			candidatesList.push_back(p2);
 		}

 		// Sort using list's member function
 		candidatesList.sort([](const SplitPlane<Objects>& a, const SplitPlane<Objects>& b) {
 			return a.position < b.position;
 		});

 		// Evaluate each candidate plane
 		for (auto& candidate : candidatesList) {
 			// Count objects on each side
 			int leftCount = 0;
 			int rightCount = 0;

 			for (auto obj : objList) {
 				BoundingBox objBB = obj->getBoundingBox();

 				// Object is on left if its min is less than split position
 				if (objBB.getMin()[axis] < candidate.position) {
 					leftCount++;
 				}
 				// Object is on right if its max is greater than split position
 				if (objBB.getMax()[axis] > candidate.position) {
    			rightCount++;
				}
 			}		

 			candidate.leftCnt = leftCount;
 			candidate.rightCnt = rightCount;

 			// Calculate left bounding box
 			candidate.leftBBox = bb;
 			glm::dvec3 leftMax = bb.getMax();
 			leftMax[axis] = candidate.position;
 			candidate.leftBBox.setMax(leftMax);

			// Calculate right bounding box
 			candidate.rightBBox = bb;
 			glm::dvec3 rightMin = bb.getMin();
 			rightMin[axis] = candidate.position;
 			candidate.rightBBox.setMin(rightMin);

 			// Calculate surface areas
 			candidate.leftBoxArea = candidate.leftBBox.area();
 			candidate.rightBoxArea = candidate.rightBBox.area();
			
 			// Calculate SAH cost
 			double parentArea = bb.area();
 			if (parentArea > 0) {
 				double cost = (leftCount * candidate.leftBoxArea + rightCount * candidate.rightBoxArea) / parentArea;

 				if (cost < minCost) {
 					minCost = cost;
 					best = candidate;
 				}
 			}
 		}
 	}

 	return best;
}

template <typename Objects>
bool SplitNode<Objects>::findIntersection(ray &r, isect &i, double &tMin, double &tMax) {
 	const glm::dvec3& origin = r.getPosition();
 	const glm::dvec3& dir = r.getDirection();

 	// Check if ray is parallel to splitting plane
 	if (abs(dir[axis]) < RAY_EPSILON) {
 		// Ray is parallel to splitting plane - check which side the ray is on
 		if (origin[axis] < position) {
 			return leftChild->findIntersection(r, i, tMin, tMax);
 		} else {
 		return rightChild->findIntersection(r, i, tMin, tMax);
 		}
 	}

 	// Calculate intersection parameter with splitting plane
 	double tSplit = (position - origin[axis]) / dir[axis];

 	// Determine which child is near and which is far based on ray origin
 	kdTreeNodes<Objects>* first;
 	kdTreeNodes<Objects>* second;

 	// Check which side of the plane the ray origin is on
 	if (origin[axis] < position) {
 		// Origin is on left side
 		first = leftChild;
 		second = rightChild;
 	} else {
 		// Origin is on right side
 		first = rightChild;
 		second = leftChild;
 	}

 	// Case 1: Split plane is beyond tMax or behind ray origin
 	if (tSplit > tMax || tSplit < 0) {
 		// Ray interval entirely on near side
 		return first->findIntersection(r, i, tMin, tMax);
 	}
 	// Case 2: Split plane is before tMin
 	else if (tSplit < tMin) {
 		// Ray interval entirely on far side
 		return second->findIntersection(r, i, tMin, tMax);
 	}
 	// Case 3: Split plane is within [tMin, tMax]
 	else {
 		// Ray crosses the splitting plane - traverse both sides
 		// Try near child first with range [tMin, tSplit]
 		if (first->findIntersection(r, i, tMin, tSplit)) {
 			return true;
 		}
 		// If no hit in near child, try far child with range [tSplit, tMax]
 		return second->findIntersection(r, i, tSplit, tMax);
 	}
}

template <typename Objects>
bool LeafNode<Objects>::findIntersection(ray &r, isect &i, double &tMin, double &tMax) {
	bool hit = false;
 	isect closestIsect;
 	double closestT = tMax;

 	// Test all objects in leaf
 	for (auto element : objList) {
 		isect currentIsect;
 		if (element->intersect(r, currentIsect)) {
 			double t = currentIsect.getT();
 			// Check if intersection is within valid range and closer than current best
 			if (t >= tMin && t < closestT) {
 				closestT = t;
 				closestIsect = currentIsect;
 				hit = true;
				if (r.type() == ray::SHADOW) {
					break; // small optimiztion
				}
 			}
 		}
 	}

 	// Update output parameters only if we found a hit
 	if (hit) {
 		i = closestIsect;
 		tMax = closestT;
 	}

 	return hit;
}