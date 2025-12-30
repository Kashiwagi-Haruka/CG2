#pragma once
#include "Object3d.h"
#include <memory>
#include "Transform.h"
#include "Camera.h"
class PlayerSkill {

	private:

	std::unique_ptr<Object3d> debugBox_;
	std::unique_ptr<Object3d> debugDamageBox1_;
	std::unique_ptr<Object3d> debugDamageBox2_;
	
	Transform transform_;
	Transform damageTransform1_;
	Transform damageTransform2_;
	Camera* camera_ = nullptr;

	float downstartposY = 5;
	
	bool isSkillEnd=false;
	int skillTime=0;
	int skillTimeMax = 60;
	float upTime = 0;
	float middleTime = 0;
	float downTime = 0;
	float damageTime = 0;

	enum State{
		up,
		middle,
		down,
		damage,
	};
	State state;
	public:

		PlayerSkill();
	    void Initialize();
	    void Update();
	    void Draw();
	    void SetCamera(Camera* camera) { camera_ = camera; }
	    void StartAttack(const Transform& playerTransform);
	    bool IsSkillEnd() { return isSkillEnd; }
};

