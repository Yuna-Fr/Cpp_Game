#pragma once
#include "Entity.hpp"

class Player : public Entity
{
public:
	// Animation constants
	static constexpr int	SCALE = 2;
	static constexpr int	FRAME_COUNT = 28;
	static constexpr int	FRAME_HEIGHT = 90;
	static constexpr int	FRAME_WIDTH = 2520 / FRAME_COUNT;

	float					inputX = 0.f;

	// Friction
	float					frx = 0.5f;
	float					fry = 1.0f;

	bool					jumping = false;
	float					recoil = 0.f;

private:
	static constexpr float	RECOIL_FORCE = 0.1f;

	// Animation Spritesheet Ranges
	static constexpr int	IDLE_S = 0;
	static constexpr int	IDLE_E = 5;
	static constexpr int	WALK_S = 6;
	static constexpr int	WALK_E = 12;
	static constexpr int	SHOOT_S = 13;	
	static constexpr int	SHOOT_E = 18;
	static constexpr int	DIE_S = 18;
	static constexpr int	DIE_E = 28;

	bool					shooting = false;

public:
	Player(std::shared_ptr<sf::Sprite> sprite, float x, float y);

	void update(double dt) override;
	void shoot();
	void jump();
	bool im() override;

private:
	void animate(float dt);
};