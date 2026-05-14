#pragma once
#include "Engine/Texture/Mesh/Animation/Skeleton.h"
#include "Engine/Texture/Mesh/Animation/SkinCluster.h"
#include "Engine/Texture/Mesh/Object3d/Object3d.h"
#include "transform.h"
#include <memory>
#include <vector>
class Camera;
class RotatingPlaygroundEquipment {

	std::unique_ptr<Object3d> spinObj_;
	std::unique_ptr<Skeleton> spinSkeleton_;
	SkinCluster spinSkinCluster_;
	const Animation::AnimationData* spinAnimation_ = nullptr;
	float spinAnimationTime_ = 0.0f;
	bool spinLoopAnimation_ = true;
	std::vector<std::unique_ptr<Object3d>> gentlemanObj_;
	std::vector<std::unique_ptr<Skeleton>> gentlemanSkeletons_;
	std::vector<SkinCluster> gentlemanSkinClusters_;
	const Animation::AnimationData* gentlemanRoundAnimation_ = nullptr;
	float gentlemanAnimationTime_ = 0.0f;
	bool gentlemanLoopAnimation_ = true;
	Transform spinTransform_;
	std::vector<Transform> gentlemanTransform_;
	float gentlemanOrbitAngle_ = 0.0f;
	float gentlemanOrbitSpeed_ = 0.02f;
	float gentlemanOrbitRadius_ = 2.0f;

	Camera* camera_ = nullptr;

public:
	RotatingPlaygroundEquipment();
	~RotatingPlaygroundEquipment();
	void Initialize();
	void Update();
	void Draw();
	void SetCamera(Camera* camera);
};