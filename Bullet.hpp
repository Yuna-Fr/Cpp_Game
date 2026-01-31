#pragma once
#include "Entity.hpp"

class Bullet : public Entity
{
public:
	static constexpr float	SPEED = 20.0f;

	// Animation constants
	static constexpr float	SCALE = 2.0f;
	static constexpr int	FRAME_COUNT = 4;
	static constexpr int	FRAME_WIDTH = 64 / FRAME_COUNT;
	static constexpr int	FRAME_HEIGHT = 16;

public:
	Bullet(std::shared_ptr<sf::Sprite> sprite, float x, float y, bool faceRight);

	void update(double dt) override;
	bool im() override;

private:
	void animate(float dt);
};