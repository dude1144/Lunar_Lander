#pragma once
//  Kevin M. Smith - CS 134 SJSU

#include "ofMain.h"
#include "Particle.h"


//  Pure Virtual Function Class - must be subclassed to create new forces.
//
class ParticleForce {
protected:
public:
	bool applyOnce = false;
	bool applied = false;
	virtual void updateForce(Particle *) = 0;
};

class ParticleSystem {
public:
	void add(const Particle &);
	void addForce(ParticleForce *);
	void removeForces() { forces.clear(); }
	void remove(int);
	void update();
	void test(Particle* p);
	void setLifespan(float);
	void reset();
	int removeNear(const glm::vec3 & point, float dist);
	void draw();
	vector<Particle> particles;
	vector<ParticleForce *> forces;
};



// Some convenient built-in forces
//
class GravityForce: public ParticleForce {
	glm::vec3 gravity;
public:
	void set(const glm::vec3 &g) { gravity = g; }
	GravityForce(const glm::vec3 & gravity);
	GravityForce() {}
	void updateForce(Particle *);
};

class TurbulenceForce : public ParticleForce {
	glm::vec3 tmin, tmax;
public:
	void set(const glm::vec3 &min, const glm::vec3 &max) { tmin = min; tmax = max; }
	TurbulenceForce(const glm::vec3 & min, const glm::vec3 &max);
	TurbulenceForce() { tmin = glm::vec3(0, 0, 0); tmax = glm::vec3(0, 0, 0); }
	void updateForce(Particle *);
};

class ImpulseForce : public ParticleForce 
{
	float magnitude = 1.0;
	glm::vec3 dir = glm::vec3(0, 0, 0);
public:
	void setMagnitude(float mag) { magnitude = mag; }
	void setDirection(glm::vec3 dir) { this->dir = dir; }
	ImpulseForce(float magnitude, glm::vec3 dir);
	ImpulseForce() {}
	void updateForce(Particle *);
};

class ImpulseRadialForce : public ParticleForce {
	float magnitude = 1.0;
	float height = .2;
public:
	void set(float mag) { magnitude = mag; }
	void setHeight(float h) { height = h; }
	ImpulseRadialForce(float magnitude);
	ImpulseRadialForce() {}
	void updateForce(Particle *);
};

class CyclicForce : public ParticleForce {
	float magnitude = 1.0;
public:
	void set(float mag) { magnitude = mag; }
	CyclicForce(float magnitude);  
	CyclicForce() {}
	void updateForce(Particle *);
};

