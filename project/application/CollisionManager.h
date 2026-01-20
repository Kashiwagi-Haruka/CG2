#pragma once

class EnemyManager;
class House;
class Player;

class CollisionManager {
public:
	void HandleGameSceneCollisions(Player& player, EnemyManager& enemyManager, House& house);
};