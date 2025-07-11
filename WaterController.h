#pragma once  
#include <array>  
#include <random>  
#include <vector>  
#include "GameBase.h"  
#include "VertexData.h"  
#include "Function.h"

class WaterController {  
public:  

  void Initialize();  
  void Update();  
  void Draw(GameBase& gamebase);  
  void StartCharge(const Vector2& mousePos, bool justPressed);  
  void Fire();  
  Vector2 ProjectToScreen(const Vector3& w, const Vector3& cam, const Vector3& tgt);  
  void CreateMetaballMesh(std::vector<VertexData>& outVertices, std::vector<uint32_t>& outIndices);  
  void DrawMetaballImGui();

  private:  

    Function fn; 
  // 分割数  
  static const int kRowCount = 12; // 縦分割（行数）  
  static const int kColCount = 50; // 横分割（列数）  

  int handle_;        // 水球テクスチャ  
  int handle2_;       // 水流テクスチャ  
  int splashTex = -1; // しぶきテクスチャ  

  Vector2 mousePos_;  
  bool isCharging_ = false;  
  bool isFired_ = false;  

  static const int gridNum_ = 3;  
  static const int texSize_ = 512;  
  static const int pieceW_ = texSize_ / gridNum_;  
  static const int pieceH_ = texSize_ / gridNum_;  

  float camDistance_ = 800.0f;  
  float camAzimuth_ = 0.0f;   // 水平角（ラジアン, 0で右、水平方向に回転）  
  float camElevation_ = 0.2f; // 上下角（ラジアン, 0で真横、π/2で真上）  
  Vector3 camTarget_ = {640.0f, 360.0f, 0.0f};  
  Vector3 camPos_; // 計算で自動生成  

  // 格子点配列: [行][列]  
  std::vector<std::vector<Vector3>> gridPoints3D_;  

  struct SplashParticle {  
      Vector3 pos3;  
      Vector3 vel3;  
      float lifetime;  
      float age;  
      float scale;  
      float angle;  
  };  

  struct MetaBall {  
      Vector3 pos;  
      float radius;  
  };  
  std::vector<MetaBall> balls = {
	  {{48, 48, 48}, 20.0f},
      {{78, 48, 48}, 20.0f}
  };

  std::vector<SplashParticle> splashes;  
  float metaBallConnectThreshold = 35.0f; // 合成/分離のしきい値（例）
};
