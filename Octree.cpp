#include "Octree.h"
 

// draw Octree (recursively)
//
void Octree::draw(TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;
	drawBox(node.box);
	level++;
	for (int i = 0; i < node.children.size(); i++) {
		draw(node.children[i], numLevels, level);
	}
}

// draw only leaf Nodes
//
void Octree::drawLeafNodes(TreeNode & node) 
{
	if (node.children.size() == 0)
		drawBox(node.box);
	else
		for (int i = 0; i < node.children.size(); i++)
			drawLeafNodes(node.children[i]);
}


//draw a box from a "Box" class  
//
void Octree::drawBox(const Box &box) {
	glm::vec3 min = box.parameters[0];
	glm::vec3 max = box.parameters[1];
	glm::vec3 size = max - min;
	glm::vec3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x, center.y, center.z);
	float w = size.x;
	float h = size.y;
	float d = size.z;
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box Octree::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	cout << "vertices: " << n << endl;
//	cout << "min: " << min << "max: " << max << endl;
	return Box(glm::vec3(min.x, min.y, min.z), glm::vec3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int Octree::getMeshPointsInBox(const vector<int>& points, Box & box, vector<int> & pointsRtn)
{
	int count = 0;

	for (int i = 0; i < points.size(); i++)
	{
		if (box.inside(this->mesh.getVertex(points[i])))
		{
			pointsRtn.push_back(points[i]);
			count++;
		}
	}

	return count;
}



//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void Octree::subDivideBox8(const Box &box, vector<Box> & boxList) {
	glm::vec3 min = box.parameters[0];
	glm::vec3 max = box.parameters[1];
	glm::vec3 size = max - min;
	glm::vec3 center = size / 2 + min;
	float xdist = (max.x - min.x) / 2;
	float ydist = (max.y - min.y) / 2;
	float zdist = (max.z - min.z) / 2;
	glm::vec3 h = glm::vec3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + glm::vec3(xdist, 0, 0), b[0].max() + glm::vec3(xdist, 0, 0));
	b[2] = Box(b[1].min() + glm::vec3(0, 0, zdist), b[1].max() + glm::vec3(0, 0, zdist));
	b[3] = Box(b[2].min() + glm::vec3(-xdist, 0, 0), b[2].max() + glm::vec3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

void Octree::create(const ofMesh & mesh, int numLevels) 
{
	// initialize octree structure
	this->mesh = mesh;
	// initialize the firt root node (level 0)
	// it contains all vertex indices from the mesh
	//
	int level = 0;
	this->root.box = meshBounds(mesh);
	for (int i = 0; i < mesh.getNumVertices(); i++) 
	{
		root.points.push_back(i);
	}
	float t1 = ofGetElapsedTimeMillis();
	// recursively buid octree (starting at level 1)
	//
	level++;
	subdivide(root, numLevels, level);
	float t2 = ofGetElapsedTimeMillis();
	cout << "Time to Build Octree: " << t2 - t1 << " milliseconds" << endl;
}

void Octree::subdivide(TreeNode & node, int numLevels, int level)
{
	if (level >= numLevels) return;

	vector<Box> boxList;
	subDivideBox8(node.box, boxList);
	level++;
	for (int i = 0; i < boxList.size(); i++) 
	{
		TreeNode child;
		int count = getMeshPointsInBox(node.points, boxList[i], child.points);
		if (count > 0) {
			child.box = boxList[i];
			node.children.push_back(child);
			if (count > 1) {
				subdivide(node.children[node.children.size() - 1], numLevels, level);
			}
		}
	}
}

bool Octree::intersect(glm::vec3 point, glm::vec3 dir, const TreeNode & node, TreeNode* nodeRtn) 
{
	//check if ray intersects this node
	if (node.box.intersect(point, dir, -1000, 1000))
	{
		//if it does, check if this is a leaf node
		if (node.children.size() == 0)
		{
			//if it is, check if this node is closer than current nodeRtn, save it if so and return true
			if (glm::length(nodeRtn->box.center() - point) > glm::length(node.box.center() - point))
				*nodeRtn = node;
			return true;
		}
		else
		{
			//if this isn't a leaf node, intersect all children and return true if any of them do
			bool ret = false;
			for (int i = 0; i < node.children.size(); i++)
			{
				ret = ret || intersect(point, dir, node.children[i], nodeRtn);
			}
			return ret;
		}
	}
	//return false if ray does not intersect this node
	return false;
}

bool Octree::intersect(glm::vec3 point, const TreeNode & node, glm::vec3* norm) const
{
	//check in point is inside current node
	const glm::vec3 p = point;
	if ((p.x >= node.box.parameters[0].x && p.x <= node.box.parameters[1].x) &&
		(p.y >= node.box.parameters[0].y && p.y <= node.box.parameters[1].y) &&
		(p.z >= node.box.parameters[0].z && p.z <= node.box.parameters[1].z))
	{
		//if it does, check if this is a leaf node
		if (node.children.size() == 0)
		{
			int index = -1;
			float dist = 1000000;
			for (int i = 0; i < node.points.size(); i++)
			{
				if (glm::length(point - mesh.getVertex(node.points[i])) < dist)
				{
					index = i;
					dist = glm::length(point - mesh.getVertex(node.points[i]));
				}
			}

			if(dist < glm::length(*norm))
				*norm = glm::normalize(mesh.getNormal(index)) * dist;
			return true;
		}
		else
		{
			//if this isn't a leaf node, intersect all children and return true if any of them do
			bool ret = false;
			for (int i = 0; i < node.children.size(); i++)
			{
				ret = ret || intersect(point, node.children[i], norm);
			}
			return ret;
		}
	}
	//return false if ray does not intersect this node
	return false;
}


