#include "Particles.h"
#include "ParticleManager.h"

Particles::Particles(){

	ParticleManager::GetInstance()->CreateParticleGroup("player", "Resources/2d/defaultParticle.png");
	ParticleManager::GetInstance()->CreateParticleGroup("leaf", "Resources/2d/leaf.png");
	ParticleManager::GetInstance()->CreateParticleGroup("goal", "Resources/2d/goalParticle.png");
	particleplayer = new ParticleEmitter(
	    "player",
	    {
	        {0.1f, 0.1f, 1.0f},
            {0.0f, 0.0f, 0.0f},
            {0,    0,    0   }
    },
	    1, 5, {-0.01f, 0.001f, 0}, {-2.5f, 0, 0}, {2.5f, 3, 1});
	particleleaf = new ParticleEmitter(
	    "leaf",
	    {
	        {0.5f, 0.5f, 1.0f},
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f}
    },
	    10, 1, {0.0f, -0.01f, 0}, {-640, -320, 0}, {640, 320, 1});
	particlegoal = new ParticleEmitter(
	    "goal",
	    {
	        {0.1f, 0.1f, 1.0f},
            {0.0f, 0.0f, 0.0f},
            {0,    0,    0   }
    },
	    10, 2, {0, 0.01f, 0}, {-0.5f, 0, 0}, {0.5f, 3, 1});
}
Particles::~Particles(){

	delete particlegoal;
	delete particleleaf;
	delete particleplayer;



}
void Particles::Update(){
	particleplayer->Update({
	    {0.1f, 0.1f,       1.0f      },
        {0,    0,          0         },
        {playerPos_.x, playerPos_.y - 0.5f, playerPos_.z - 1.0f}
    });

	particleleaf->Update({
	    {0.1f,	                                     0.1f,	                                     1.0f},
        {0,	                                        0,	                                        0   },
        {cameraPos_.x, cameraPos_.y, 0   }
    });
}
void Particles::SetPlayerPos(Vector3 playerPos){ playerPos_ = playerPos; }
void Particles::SetCameraPos(Vector3 cameraPos) { cameraPos_ = cameraPos; }