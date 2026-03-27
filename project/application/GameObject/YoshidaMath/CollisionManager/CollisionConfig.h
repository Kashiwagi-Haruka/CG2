#pragma once
#include <cstdint>

namespace {
    //プレイヤー陣営
    constexpr uint32_t kCollisionPlayer = 0b1;
    //床
    constexpr uint64_t kCollisionFloor = 0b1 << 1;
    //アイテム
    constexpr uint64_t kCollisionItem = 0b1 << 2;
    //ポータル
    constexpr uint32_t kCollisionPortal = 0b1 << 3;
    //敵陣営
    constexpr uint32_t kCollisionEnemy = 0b1 << 4;
    //椅子
    constexpr uint64_t kCollisionChair = 0b1 << 5;
    //壁
    constexpr uint64_t kCollisionWall = 0b1 << 6;
    //自販機
    constexpr uint64_t kCollisionVendingMac = 0b1 << 7; 
    //ドア
    constexpr uint64_t kCollisionDoor = 0b1 << 8;
    //鍵
    constexpr uint64_t kCollisionKey = 0b1 << 9;
    //マット
    constexpr uint64_t kCollisionMat = 0b1 << 10;
    //ロッカー
    constexpr uint64_t kCollisionLocker = 0b1 << 11;
    //机
    constexpr uint64_t kCollisionDesk = 0b1 << 12;
    //何の陣営にも属さない
    constexpr uint32_t kCollisionNone = 0b0;
}
