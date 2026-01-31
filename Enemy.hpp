#pragma once
#include "Entity.hpp"

class Enemy : public Entity
{
public:
    static constexpr float  SPEED = 3.0f;

	// Animation constants
    static constexpr float  SCALE = 2.7f;
    static constexpr int    FRAME_COUNT = 6;
    static constexpr int    FRAME_WIDTH = 32;
    static constexpr int    FRAME_HEIGHT = 138 / FRAME_COUNT;

    Enemy(std::shared_ptr<sf::Sprite> sprite, float x, float y);

    void update(double dt) override;
    bool im() override;
	void dies() override;

private:
    void animate(float dt);
};