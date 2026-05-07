
#pragma once
#include <stdint.h> // uint32_tを使うため

class ItemBase {
public:
    virtual ~ItemBase() = default;
    // インベントリのUIに表示するテクスチャハンドルを取得するための仮想関数
    virtual uint32_t GetTextureHandle() const = 0;
    // ▼ 追加：アイテムの種類を判別するためのIDを取得する
    virtual int GetItemID() const = 0;
};