#pragma once

class EnemyManager;
class ExpCubeManager;
class House;
class Player;

class CollisionManager {
public:
	void HandleGameSceneCollisions(Player& player, EnemyManager& enemyManager, ExpCubeManager& expCubeManager, House& house);
};