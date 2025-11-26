#pragma once
#include "ParticleEmitter.h"

class Particles {

	ParticleEmitter* particleplayer = nullptr;
	ParticleEmitter* particleleaf = nullptr;
	ParticleEmitter* particlegoal = nullptr;

	Vector3 playerPos_;
	Vector3 cameraPos_;

	public:

	Particles();
	~Particles();
	void Update();
	void SetPlayerPos(Vector3 playerPos);
	void SetCameraPos(Vector3 cameraPos);
};
