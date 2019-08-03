#pragma once
#include "ofMain.h"
#include "box.h"


class TreeNode {
public:
	Box box;
	vector<int> points;
	vector<TreeNode> children;
};

class Octree 
{
public:
	
	void create(const ofMesh & mesh, int numLevels);
	void subdivide(TreeNode & node, int numLevels, int level);
	bool intersect(glm::vec3 point, glm::vec3 dir, const TreeNode & node, TreeNode* nodeRtn);
	bool intersect(glm::vec3 point, const TreeNode & node, glm::vec3* norm) const;
	void draw(TreeNode & node, int numLevels, int level);
	void draw(int numLevels, int level) { draw(root, numLevels, level); }
	void drawLeafNodes(TreeNode & node);
	static void drawBox(const Box &box);
	static Box meshBounds(const ofMesh &);
	int getMeshPointsInBox(const vector<int> & points, Box & box, vector<int> & pointsRtn);
	void subDivideBox8(const Box &b, vector<Box> & boxList);
	bool insideBox(glm::vec3 p, Box box)
	{
		return ((p.x >= box.parameters[0].x && p.x <= box.parameters[1].x) &&
			(p.y >= box.parameters[0].y && p.y <= box.parameters[1].y) &&
			(p.z >= box.parameters[0].z && p.z <= box.parameters[1].z));
	}

	ofMesh mesh;
	TreeNode root;
};