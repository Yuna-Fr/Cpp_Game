#include "Pet.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "PetBullet.hpp"

Pet::Pet(std::shared_ptr<sf::Sprite> sprite) : Entity(sprite)
{
	sprite->setTextureRect(sf::IntRect(0, 0, FRAME_WIDTH/2, FRAME_HEIGHT/2));
	sprite->setScale(SCALE, SCALE);
	sprite->setOrigin(FRAME_WIDTH * 0.5, 25);

	setPosGrid(20, 20);
	rx = 0;
	ry = 0.99f;
	syncPos();

	isFacingRight = false;
	animSpeed = 0.3f;

	player = g.player;
}

void Pet::update(double dt)
{
	animate((float)dt);
	
	if (auto p = player.lock()) 
	{
		if (p->isDead) return;
	
		// Shooting logic
		if (!bullet.lock()) {
			bulletTimer += dt;
			if (bulletTimer >= bulletReloadingTime && g.enemies.size() != 0)
			{
				// Get closest enemy if close enough
				float targetDist = MAX_ATTACK_DIST;
				std::shared_ptr<Enemy> target = nullptr;

				for (auto& e : g.enemies) 
				{
					if (e->isDead) continue;

					float dist = std::abs((e->cx + e->rx) - (cx + rx));
					if (dist < targetDist)
					{
						if (canSee(e->cx, e->cy))
						{
							targetDist = dist;
							target = e;
						}
					}
				}

				if (target) {
					bullet = g.spawnPetBullet(cx + rx, cy + ry, target);
					bulletTimer = 0.f;
				}
			}
		}

		Vector2<int> targetPos = p->getPosGrid();
		targetPos = p->isFacingRight ? targetPos + Vector2<int>{-3, -3} : targetPos + Vector2<int>{3, -3};

		if (targetPos.x == cx && targetPos.y == cy)
		{
			dx = 0;
			animSpeed = 0.5f;
			sprite->setScale(p->isFacingRight ? -abs(SCALE) : abs(SCALE), SCALE);
			return;
		}
		
		animSpeed = 0.3f;

		if (cy < targetPos.y)
			dy = SPEED;
		else if (cy > targetPos.y)
			dy = -SPEED;
		else
			dy = 0;

		if (cx < targetPos.x) {
			isFacingRight = true;
			dx = SPEED;
		}
		else if (cx > targetPos.x) {
			isFacingRight = false;
			dx = -SPEED;
		}
		else {
			isFacingRight = p->isFacingRight;
			dx = 0;
		}

		ry += dy * dt;
		rx += dx * dt;

		if (dy > 0 && ry > 2)
		{
			ry--;
			cy++;
		}
		else if (dy < 0 && ry < -1)
		{
			cy--;
			ry++;
		}

		if (rx > 1.0f || rx < 0.0f)
		{
			int nextCx = isFacingRight ? cx + 1 : cx - 1;
			float nextRx = isFacingRight ? rx - 1.f : rx + 1.f;

			cx = nextCx;
			rx = nextRx;
		}

		sprite->setScale(isFacingRight ? -abs(SCALE) : abs(SCALE), SCALE);
		syncPos();
	}
}

bool Pet::im()
{
	return false;
}

bool Pet::canSee(int targetX, int targetY) // Bresenham's line algorithm
{
	int petX = cx;
	int petY = cy;

	bool swapXY = std::abs(targetY - petY) > std::abs(targetX - petX);

	if (swapXY)
	{
		std::swap(petX, petY);
		std::swap(targetX, targetY);
	}

	if (petX > targetX)
	{
		std::swap(petX, targetX);
		std::swap(petY, targetY);
	}

	int deltaX = targetX - petX;
	int deltaY = std::abs(targetY - petY);
	int error = deltaX / 2;

	int y = petY;
	int ystep = (petY < targetY) ? 1 : -1;

	for (int x = petX; x <= targetX; ++x)
	{
		int worldX = swapXY ? y : x;
		int worldY = swapXY ? x : y;

		// Ignore starting tile, allow target tile
		if (!(worldX == cx && worldY == cy))
		{
			if (g.hasCollision(worldX, worldY))
				return false;
		}

		error -= deltaY;
		if (error < 0)
		{
			y += ystep;
			error += deltaX;
		}
	}

	return true;
}

void Pet::animate(float dt)
{
	animTimer += dt;

	if (animTimer >= animSpeed)
	{
		animTimer = 0.f;
		frame = (frame + 1) % FRAME_COUNT;
		sprite->setTextureRect({ frame * FRAME_WIDTH, 0, FRAME_WIDTH, FRAME_HEIGHT });
	}
}