#pragma once
#include "Transform.h"
#include <cstdint>
#include <memory>
#include <string>
#include "Primitive/Primitive.h"

class ParticleEmitter;

class PortalParticle {
public:
	PortalParticle();
	~PortalParticle() = default;

	void Initialize();
	void Update(const Transform& transform);
	void Draw();

private:
	std::unique_ptr<Primitive> primitive_ = nullptr;
	std::unique_ptr<ParticleEmitter> emitter_ = nullptr;
	Transform transform_{};
	static uint32_t nextId_;
};