#ifndef _BOX_H_
#define _BOX_H_

#include <assert.h>
#include "ofMain.h"

/*
 * Axis-aligned bounding box class, for use with the optimized ray-box
 * intersection test described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 *
 */

class Box {
  public:
    Box() { }
    Box(const glm::vec3 &min, const glm::vec3 &max) {
 //     assert(min < max);
      parameters[0] = min;
      parameters[1] = max;
    }
    // (t0, t1) is the interval for valid hits
    bool intersect(glm::vec3 point, glm::vec3 dir, float t0, float t1) const;
	bool intersect(glm::vec3 min, glm::vec3 max) const;

    // corners
    glm::vec3 parameters[2];
	glm::vec3 min() const { return parameters[0]; } 
	glm::vec3 max() const { return parameters[1]; }

	const bool inside(glm::vec3 p) {
		return ((p.x >= parameters[0].x && p.x <= parameters[1].x) &&
		     	(p.y >= parameters[0].y && p.y <= parameters[1].y) &&
			    (p.z >= parameters[0].z && p.z <= parameters[1].z));
	}
	const bool inside(glm::vec3 *points, int size) {
		bool allInside = true;
		for (int i = 0; i < size; i++) {
			if (!inside(points[i])) allInside = false;
			break;
		}
		return allInside;
	}

	glm::vec3 center() const
	{
		return ((max() - min()) / 2 + min());
	}
};

#endif // _BOX_H_
