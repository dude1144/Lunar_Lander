#include "box.h"
 
#define hitInterval 1000
/*
 * Ray-box intersection using IEEE numerical properties to ensure that the
 * test is both robust and efficient, as described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 *
 */

bool Box::intersect(glm::vec3 point, glm::vec3 dir, float t0, float t1) const 
{
	glm::vec3 invDir = glm::vec3(1 / dir.x, 1 / dir.y, 1 / dir.z);
	int xSign = invDir.x < 0;
	int ySign = invDir.y < 0;
	int zSign = invDir.z < 0;

	float tXMin = (parameters[xSign].x - point.x) * invDir.x;
	float tXMax = (parameters[1 - xSign].x - point.x) * invDir.x;
	float tYMin = (parameters[ySign].y - point.y) * invDir.y;
	float tYMax = (parameters[1 - ySign].y - point.y) * invDir.y;
	if (tXMin > tYMax || tYMin > tXMax)
		return false;
	if (tYMin > tXMin)
		tXMin = tYMin;
	if (tYMax < tXMax)
		tXMax = tYMax;
	float tZMin = (parameters[zSign].z - point.z) * invDir.z;
	float tZMax = (parameters[1 - zSign].z - point.z) * invDir.z;
	if ((tXMin > tZMax) || (tZMin > tXMax))
		return false;
	if (tZMin > tXMin)
		tXMin = tZMin;
	if (tZMax < tXMax)
		tXMax = tZMax;
	return ((tXMin < hitInterval) && (tXMax > -hitInterval));
}

bool Box::intersect(glm::vec3 center, glm::vec3 extents) const
{
	glm::vec3 thisExtents = glm::vec3(abs((parameters[0].x - parameters[1].x) / 2), abs((parameters[0].y - parameters[1].y) / 2), abs((parameters[0].z - parameters[1].z) / 2));
	
	if (abs(this->center().x - center.x) > (thisExtents.x + extents.x)) return false;
	if (abs(this->center().y - center.y) > (thisExtents.y + extents.y)) return false;
	if (abs(this->center().z - center.z) > (thisExtents.z + extents.z)) return false;

	return true;
}
