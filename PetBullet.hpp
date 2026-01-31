#pragma once
#include "Entity.hpp"

class PetBullet : public Entity
{
public:
	static constexpr float	SPEED = 20.0f;

	// Animation constants
	static constexpr float	SCALE = 2.0f;
	static constexpr int	FRAME_COUNT = 4;
	static constexpr int	FRAME_WIDTH = 64 / FRAME_COUNT;
	static constexpr int	FRAME_HEIGHT = 16;

private:
	std::weak_ptr<Enemy> targetEnemy;

public:
	PetBullet(std::shared_ptr<sf::Sprite> sprite, float x, float y, const std::shared_ptr<Enemy>& target);

	void update(double dt) override;
	bool im() override;

private:
	void animate(float dt);
};