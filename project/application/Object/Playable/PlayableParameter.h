#pragma once

struct PlayableParameter {

	int HPMax;//最大HP
	int AttackPower;//攻撃力
	int DefensePower;//防御力
	int AstralLinkPower;//星
	AttackStatus attack;
	bool ShadowsOfTheStars[6];
};

struct AttackStatus {

NormalAttack normal;


};

struct NormalAttack {

	float attack1Power;
	float attack2Power;
	float attack3Power;
	float attack4Power;
	float heavyAttack;
};