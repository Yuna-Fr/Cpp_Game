#pragma once
#include "Entity.hpp"

class Pet : public Entity
{
public:
	static constexpr float		SPEED = 7.0f;
	static constexpr float		MAX_ATTACK_DIST = 15.0f;

	// Animation constants
	static constexpr float		SCALE = 0.3f;
	static constexpr int		FRAME_COUNT = 2;
	static constexpr int		FRAME_WIDTH = 160 / FRAME_COUNT;
	static constexpr int		FRAME_HEIGHT = 160 / FRAME_COUNT;

private:
	float						bulletTimer = 0.f;
	float						bulletReloadingTime = 2.0f;
	std::weak_ptr<Player>		player;
	std::weak_ptr<PetBullet>	bullet;

public:
	Pet(std::shared_ptr<sf::Sprite> sprite);

	void update(double dt) override;
	bool im() override;

private:
	bool canSee(int targetX, int targetY);
	void animate(float dt);
};


