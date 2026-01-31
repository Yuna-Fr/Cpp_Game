#include <imgui.h>
#include "Enemy.hpp"

Enemy::Enemy(std::shared_ptr<sf::Sprite> sprite, float x, float y) : Entity(sprite)
{
	sprite->setTextureRect(sf::IntRect(0, 0, FRAME_WIDTH, FRAME_HEIGHT));
	sprite->setScale(SCALE, SCALE);
	sprite->setOrigin(FRAME_WIDTH / 2, FRAME_HEIGHT - 2.f);

	setPosGrid(x, y);
	ry = 0.99f;
	syncPos();
}

void Enemy::update(double dt)
{
    dx = (isFacingRight ? SPEED : -SPEED);
    dy += gravity * dt;

	rx += dx * dt;
    ry += dy * dt;

    // Horizontal Movements
    if (rx > 1.0f || rx < 0.0f) 
    {
        int nextCx = isFacingRight ? cx + 1 : cx - 1;
        float nextRx = isFacingRight ? rx - 1.f : rx + 1.f;

        if (g.hasCollision(nextCx + rx, cy) || g.hasCollision(nextCx + rx, cy - 1.f))
        {
            dx = 0;
            rx = isFacingRight ? 0.99f : 0;
            isFacingRight = !isFacingRight;
            sprite->setScale(isFacingRight ? abs(SCALE) : -abs(SCALE), SCALE);
        }
        else
        {
            setGravity(true);
            cx = nextCx;
            rx = nextRx;
        }
    }

	if ((dy > 0)) // if is falling
	{
		if (g.hasCollision(cx + rx, cy + ry))
		{
			setGravity(false);
			ry = 0.99f;
			dy = 0;
		}
		else
		{
			setGravity(true);
			if (ry > 1)
			{
				ry--;
				cy++;
			}
		}
	}

	syncPos();
	animate(dt);
}

void Enemy::animate(float dt)
{
	animTimer += dt;

	if (animTimer >= animSpeed)
	{
		animTimer = 0.f;
		frame = (frame + 1) % FRAME_COUNT;
		sprite->setTextureRect({ 0, frame * FRAME_HEIGHT, FRAME_WIDTH, FRAME_HEIGHT });
	}
}

bool Enemy::im()
{
    using namespace ImGui;

    Value("cx", cx);
    Value("rx", rx);
    Value("cy", cy);
    Value("ry", ry);
	Value("Enemy directionRight", isFacingRight);

    return true;
}

void Enemy::dies()
{
	Entity::dies();
	isDead = true;
}
