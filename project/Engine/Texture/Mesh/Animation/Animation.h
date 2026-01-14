#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <vector>
#include <string>
#include <map>
class Animation {

	struct KeyframeVector3 {
		Vector3 value;
		float time;
	};

	struct KeyframeVector4 {
		Vector4 value;
		float time;
	};

	template<typename tValue> 
	struct Keyflame {
		float time;
		tValue value;
	
	};

	using KeyframeVector3 = Keyflame<Vector3>;
	using KeyframeVector4 = Keyflame<Vector4>;

	struct NodeAnimation {
		std::vector<KeyframeVector3> translate;
		std::vector<KeyframeVector4> rotation;
		std::vector<KeyframeVector3> scale;
	};
	template<typename tValue>
	struct AnimationCurve{
		std::vector<Keyflame<tValue>> keyframes;
	};
	struct NodeAnimation{
		AnimationCurve<Vector3> translate;
		AnimationCurve<Vector4> rotation;
		AnimationCurve<Vector3> scale;
	};

	struct AnimationData {
		float duration;
		
		std::map<std::string,NodeAnimation> nodeAnimations;
	};
};
