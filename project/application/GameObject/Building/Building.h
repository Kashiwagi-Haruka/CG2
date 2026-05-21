#pragma once
#include "Object3d/InstancedObject3d/InstancedObject3d.h"
#include "Vector3.h"
#include <memory>
#include <vector>
#include <string>



class BuildingClass {

public:
    struct Datas{
        Vector3 pos;
        float rotY;
    };
private:
    std::unique_ptr<InstancedObject3d> instancedObject_;
    std::vector<Datas> datas_;
public:
    BuildingClass() = default;
    ~BuildingClass() = default;
    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="modelPath">建物モデルのファイルパス</param>
    void Initialize(const std::string& modelPath, const std::vector<Datas>& datas);

    /// <summary>
    /// 更新
    /// </summary>
    void Update(const uint32_t floorNum = 1);

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    /// @brief カメラ
    /// @param camera 
    void SetCamera(Camera* camera);

    /// <summary>
    /// 建物の配置座標を動的に再設定する
    /// </summary>
    void SetBuildingPositions(const std::vector<Datas>& datas);
};