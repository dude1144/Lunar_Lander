#pragma once

#include "ofMain.h"

class ParticleForceField;

class Particle 
{
public:
	Particle();

	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 forces;
	float	damping;
	float   mass;
	float   lifespan;
	float   radius;
	float   birthtime;
	void    integrate();
	void    draw();
	float   age();        // sec
	ofColor color;
};


