#include "PetBullet.hpp"
#include "Enemy.hpp"
#include "C.hpp"

PetBullet::PetBullet(std::shared_ptr<sf::Sprite> sprite, float x, float y, const std::shared_ptr<Enemy>& target) : Entity(sprite)
{
	sprite->setTextureRect(IntRect(0, 0, FRAME_WIDTH, FRAME_HEIGHT));
	sprite->setScale(SCALE, SCALE);
	sprite->setOrigin(0, FRAME_HEIGHT);

	targetEnemy = target;
	setPosGrid(x, y);
	ry = 0.99f;
	syncPos();
}

void PetBullet::update(double dt)
{
	if (isDead) return;

	if (waitToDie) 
	{
		animate(dt);
		return;
	}

	auto enemy = targetEnemy.lock();
	if (!enemy)
	{
		waitToDie = true;
		return;
	}

	Vector2<int> targetPos = enemy->getPosGrid();

	if (targetPos.x == cx && targetPos.y == cy)
	{
		dx = dy = 0.0f;
		return;
	}

	dx = (targetPos.x > cx) ? SPEED : (targetPos.x < cx ? -SPEED : 0.0f);
	dy = (targetPos.y > cy) ? SPEED : (targetPos.y < cy ? -SPEED : 0.0f);

	ry += dy * dt;
	rx += dx * dt;

	if (rx > 1.0f) {
		rx--;
		cx++;
	}
	else if (rx < 0.0f) {
		rx++;
		cx--;
	}

	if (ry > 2.0f) {
		ry--;
		cy++;
	}
	else if (ry < -1.0f) {
		ry++;
		cy--;
	}

	float bx = (cx + rx) * C::GRID_SIZE;
	float by = (cy + ry) * C::GRID_SIZE;

	float tx = (enemy->cx + enemy->rx) * C::GRID_SIZE;
	float ty = (enemy->cy + enemy->ry) * C::GRID_SIZE;

	float angle = std::atan2((ty - by), (tx - bx)) * 180.f / 3.14f;

	sprite->setRotation(angle);

	syncPos();
}

bool PetBullet::im() { return false; }

void PetBullet::animate(float dt)
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