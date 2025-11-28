#pragma once
#include "ParticleEmitter.h"

class Particles {

	ParticleEmitter* particleplayer = nullptr;
	ParticleEmitter* particleleaf = nullptr;
	ParticleEmitter* particlegoal = nullptr;
	

	Vector3 playerPos_;
	Vector3 cameraPos_;
	Vector3 goalPos_;
	Transform playerEmitterTransform;

	public:

	Particles();
	~Particles();
	void Update();
	void SetPlayerPos(Vector3 playerPos);
	void SetCameraPos(Vector3 cameraPos);
	void SetGoalPos(Vector3 goalPos);
	void EditSingleEmitter(ParticleEmitter* e);
};
