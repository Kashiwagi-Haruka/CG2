#pragma once
#include "BaseStage.h"
#include <memory>
#include <string>

class Player;
class ElevatorRoomManager;
class PlayerCamera;
class CollisionManager;

class StageManager {
private:
	std::unique_ptr<BaseStage> stage_ = nullptr;
	std::unique_ptr<BaseStage> nextStage_ = nullptr;
	Player* player_ = nullptr;
	PlayerCamera* playerCamera_ = nullptr;
	ElevatorRoomManager* elevatorRoomManager_ = nullptr;
	CollisionManager* collisionManager_ = nullptr;
	bool useDirectionalShadow_ = true;
	std::string currentStageName_;
	std::string nextStageName_;

	std::unique_ptr<BaseStage> CreateStage(const std::string& stageName) const;

public:
	void Finalize();
	void Update();
	void CheckCollision();
	void ShadowMapDraw();
	void MainDraw();

	void SetPlayer(Player* player) { player_ = player; }
	void SetPlayerCamera(PlayerCamera* playerCamera) { playerCamera_ = playerCamera; }
	void SetElevatorManager(ElevatorRoomManager* elevatorRoomManager) { elevatorRoomManager_ = elevatorRoomManager; }
	void SetCollisionManager(CollisionManager* collisionManager) { collisionManager_ = collisionManager; }
	void SetDirectionalShadowEnabled(bool enabled);
	void ChangeStage(const std::string& stageName);
	bool IsCurrentEventRunning() const;

	const std::string& GetCurrentStageName() const { return currentStageName_; }
	float GetPlayerHp() const;
	float GetPlayerMaxHp() const;
	bool DidPlayerTakeDamage() const;
	bool IsPlayerDead() const;
};