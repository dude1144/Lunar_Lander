
// Kevin M.Smith - CS 134 SJSU

#include "ParticleSystem.h"

void ParticleSystem::add(const Particle &p) {
	particles.push_back(p);
}

void ParticleSystem::addForce(ParticleForce *f) {
	f->applied = false;
	forces.push_back(f);
}

void ParticleSystem::remove(int i) {
	particles.erase(particles.begin() + i);
}

void ParticleSystem::setLifespan(float l) {
	for (int i = 0; i < particles.size(); i++) {
		particles[i].lifespan = l;
	}
}

void ParticleSystem::reset() {
	for (int i = 0; i < forces.size(); i++) {
		forces[i]->applied = false;
	}
}

void ParticleSystem::update() {
	// check if empty and just return
	if (particles.size() == 0) return;

	vector<Particle>::iterator p = particles.begin();
	vector<Particle>::iterator tmp;

	// check which particles have exceed their lifespan and delete
	// from list.  When deleting multiple objects from a vector while
	// traversing at the same time, we need to use an iterator.
	//
	while (p != particles.end()) {
		if (p->lifespan != -1 && p->age() > p->lifespan) {
			tmp = particles.erase(p);
			p = tmp;
		}
		else p++;
	}

	// update forces on all particles first 
	//
	for (int i = 0; i < particles.size(); i++) {
		for (int k = 0; k < forces.size(); k++) {
			if (!forces[k]->applied)
				forces[k]->updateForce( &particles[i] );
		}
	}

	// update all forces only applied once to "applied"
	// so they are not applied again.
	//
	for (int i = 0; i < forces.size(); i++) 
	{
		if (forces[i]->applyOnce)
		{
			delete forces[i];
			forces.erase(forces.begin() + i);
			i--;
		}
	}

	// integrate all the particles in the store
	//
	for (int i = 0; i < particles.size(); i++)
		particles[i].integrate();

}

void ParticleSystem::test(Particle* p)
{
	for (int k = 0; k < forces.size(); k++)
	{
		if (!forces[k]->applied)
			forces[k]->updateForce(p);
	}
	p->integrate();
}

// remove all particlies within "dist" of point (not implemented as yet)
//
int ParticleSystem::removeNear(const glm::vec3 & point, float dist) { return 0; }

//  draw the particle cloud
//
void ParticleSystem::draw() {
	for (int i = 0; i < particles.size(); i++) {
		particles[i].draw();
	}
}


// Gravity Force Field 
//
GravityForce::GravityForce(const glm::vec3 &g) {
	gravity = g;
}

void GravityForce::updateForce(Particle * particle) {
	//
	// f = mg
	//
	particle->forces += gravity * particle->mass;
}

// Turbulence Force Field 
//
TurbulenceForce::TurbulenceForce(const glm::vec3 &min, const glm::vec3 &max) {
	tmin = min;
	tmax = max;
}

void TurbulenceForce::updateForce(Particle * particle) {
	//
	// We are going to add a little "noise" to a particles
	// forces to achieve a more natual look to the motion
	//
	particle->forces.x += ofRandom(tmin.x, tmax.x);
	particle->forces.y += ofRandom(tmin.y, tmax.y);
	particle->forces.z += ofRandom(tmin.z, tmax.z);
}

// Impulse Radial Force - this is a "one shot" force that
// eminates radially outward in random directions.
//
ImpulseForce::ImpulseForce(float magnitude, glm::vec3 dir) 
{
	this->magnitude = magnitude;
	this->dir = dir;
	applyOnce = true;
}

void ImpulseForce::updateForce(Particle * particle) 
{
	particle->forces += glm::normalize(dir) * magnitude;
}

// Impulse Radial Force - this is a "one shot" force that
// eminates radially outward in random directions.
//
ImpulseRadialForce::ImpulseRadialForce(float magnitude) {
	this->magnitude = magnitude;
	applyOnce = true;
}

void ImpulseRadialForce::updateForce(Particle * particle) {

	// we basically create a random direction for each particle
	// the force is only added once after it is triggered.
	//
	glm::vec3 dir = glm::vec3(ofRandom(-1, 1), ofRandom(-height/2.0, height/2.0), ofRandom(-1, 1));
	particle->forces += glm::normalize(dir) * magnitude;
}

CyclicForce::CyclicForce(float magnitude) {
	this->magnitude = magnitude;
}

void CyclicForce::updateForce(Particle * particle) {

	glm::vec3 position = particle->position;
	glm::vec3 norm = glm::normalize(position);
	glm::vec3 dir = glm::cross(norm, glm::vec3(0, 1, 0));
	particle->forces += glm::normalize(dir) * magnitude;
}

