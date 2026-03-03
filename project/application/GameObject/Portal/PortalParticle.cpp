#include "PortalParticle.h"
#include "ParticleEmitter.h"
#include "ParticleManager.h"
#include <string>

uint32_t PortalParticle::nextId_ = 0;

PortalParticle::PortalParticle() {
	ParticleManager::GetInstance()->CreateParticleGroup("portalBall", "Resources/2d/defaultParticle.png");

	emitter_ = std::make_unique<ParticleEmitter>("portalBall");
	emitter_->SetFrequency(0.15f);
	emitter_->SetCount(8);
	emitter_->SetLife(1.2f);
	emitter_->SetAcceleration({0.0f, 0.0f, 0.0f});
	emitter_->SetAreaMin({-0.45f, -0.45f, -0.45f});
	emitter_->SetAreaMax({0.45f, 0.45f, 0.45f});
	emitter_->SetBeforeColor({0.2f, 0.5f, 1.0f, 1.0f});
	emitter_->SetAfterColor({0.2f, 0.5f, 1.0f, 0.0f});
	emitter_->SetEmissionAngle(2.0f * 3.1415926535f);

	primitive_ = std::make_unique<Primitive>();
	primitive_->Initialize(Primitive::Sphere, 16);

}

void PortalParticle::Initialize() {

}

void PortalParticle::Update(const Transform& transform) {
	transform_ = transform;

	if (emitter_) {
		emitter_->SetTransform(transform_);
		emitter_->Update(transform_);
	}
}

void PortalParticle::Draw() {
	if (emitter_) {
		emitter_->Draw();
	}
}