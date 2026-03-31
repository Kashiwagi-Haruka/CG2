#pragma once
class  GameEvent
{
private:
    bool isRunning_ = false;
protected:
    virtual void OnStart() = 0;
    virtual void OnEnd() = 0;
public:
    GameEvent() = default;
    virtual void Update() = 0;
    void StartEvent();
    void EndEvent();
    bool IsRunning() { return isRunning_; };
};

