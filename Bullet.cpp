#include "Bullet.hpp"

Bullet::Bullet(std::shared_ptr<sf::Sprite> sprite, float x, float y, bool faceRight) : Entity(sprite)
{
	isFacingRight = faceRight;

	sprite->setTextureRect(IntRect(0, 0, FRAME_WIDTH, FRAME_HEIGHT));
	sprite->setScale(isFacingRight ? abs(SCALE) : -abs(SCALE), SCALE);
	sprite->setOrigin(0, FRAME_HEIGHT);

	setPosGrid(x, y);
	ry = 0.99f;
	syncPos();
}

void Bullet::update(double dt)
{
	if (isDead) return;

	if (waitToDie)
	{
		animate(dt);
		return;
	}

	dx = (isFacingRight ? SPEED : -SPEED);
	rx += dx * dt;

	if (rx > 1.0f || rx < 0.0f)
	{
		int nextCx = isFacingRight ? cx + 1 : cx - 1;
		float nextRx = isFacingRight ? rx - 1.f : rx + 1.f;

		if (g.hasCollision(nextCx + rx, cy) || g.hasCollision(nextCx + rx, cy - 1.f))
		{
			dx = 0;
			waitToDie = true;
		}
		else
		{
			cx = nextCx;
			rx = nextRx;
		}
	}

	syncPos();
}

bool Bullet::im() { return false; }

void Bullet::animate(float dt)
{
	animTimer += dt;

	if (animTimer >= animSpeed)
	{
		animTimer = 0.f;
		frame = frame + 1;
		sprite->setTextureRect({ frame * FRAME_WIDTH, 0, FRAME_WIDTH, FRAME_HEIGHT });

		if (frame == FRAME_COUNT)
			isDead = true;
	}
}