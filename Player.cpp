#include <imgui.h>
#include "Player.hpp"
#include "Enemy.hpp"

Player::Player(std::shared_ptr<sf::Sprite> sprite, float x, float y) : Entity(sprite)
{
	sprite->setTextureRect(sf::IntRect(0, 0, FRAME_WIDTH, FRAME_HEIGHT));
	sprite->setScale(SCALE, SCALE);
	sprite->setOrigin(FRAME_WIDTH * 0.5, 25);

	setPosGrid(x, y);
	ry = 0.99f;
	syncPos();

	animSpeed = 0.08f;
}

void Player::update(double dt)
{
	if (isDead) return;

	double normaliziedDt = dt * 60.0f;

	if (!waitToDie && inputX != 0) // Get Input direction
	{
		isFacingRight = (inputX > 0) ? true : false;
		sprite->setScale(isFacingRight ? abs(SCALE) : -abs(SCALE), SCALE);
		dx += inputX * normaliziedDt;
	}

	dx += recoil;
	dy += gravity * dt;

	dx *= pow(frx, normaliziedDt);
	dy *= pow(fry, normaliziedDt);
	recoil *= pow(frx, normaliziedDt);

	rx += dx * dt;
	ry += dy * dt;

	animate(dt);

	if (waitToDie) return;
	
	// Horizontal Movements
	if (rx > 1.0f || rx < 0.0f)
	{
		bool directionRight = (rx > 1.0f) ? true : false;
		int nextCx = directionRight ? cx + 1 : cx;

		if (g.hasCollision(nextCx + rx, cy + ry) || g.hasCollision(nextCx + rx, cy + ry - 1))
		{
			dx = 0;
			rx = directionRight ? 0.99f : 0;
		}
		else
		{
			setGravity(true);

			if (directionRight)
			{
				cx++;
				rx--;
			}
			else
			{
				cx--;
				rx++;
			}
		}
	}

	// Vertical Movements
	if (dy > 0)
	{
		bool hitGround = false;
		int currentCell = static_cast<int>(std::floor(cx + rx));

		// if pos is between 2 cells, test the second one (to not fall through on an edge)
		if (cx + rx != currentCell) 
		{
			int secondCell = currentCell + 1;
			hitGround = g.hasCollision(currentCell, cy + ry) || g.hasCollision(secondCell, cy + ry);
		}
		else
			hitGround = g.hasCollision(currentCell, cy + ry);

		if (hitGround)
		{
			setGravity(false);
			jumping = false;
			dy = 0;
			ry = 0.99f;
		}
		else if (ry > 1)
		{
			setGravity(true);
			cy++;
			ry--;
		}
	}
	else if (dy < 0) // Jump, No collision test to be able to jump through platforms
	{
		setGravity(true);

		while (ry < 0)
		{
			cy--;
			ry++;
		}
	}

	syncPos();
}

#pragma region InputCallbacks

void Player::shoot()
{
	if (!shooting) 
	{
		g.spawnPlayerBullet(cx + rx, cy, isFacingRight);
		shooting = true;
	}

	recoil = isFacingRight ? -RECOIL_FORCE : RECOIL_FORCE;
}

void Player::jump()
{
	if (jumping) return;

	dy -= 35;
	jumping = true;
	setGravity(true);
}

#pragma endregion

void Player::animate(float dt)
{
	int start;
	int end;
	
	animTimer += dt;

	if (animTimer >= animSpeed)
	{
		animTimer = 0.f;

		// Dies
		if (waitToDie)
		{
			animSpeed = 0.08f;
			start = DIE_S;
			end = DIE_E;
		}
		// Shoot
		else if (shooting) 
		{
			animSpeed = 0.04f;
			start = SHOOT_S;
			end = SHOOT_E;
		}
		// Idle in air
		else if (jumping)
		{
			animSpeed = 0.2f;
			start = IDLE_S;
			end = IDLE_E;
		}
		// Walk
		else if (std::abs(dx) > 0.01f)
		{
			animSpeed = 0.08f;
			start = WALK_S;
			end = WALK_E;
		}
		// Idle
		else
		{
			animSpeed = 0.2f;
			start = IDLE_S;
			end = IDLE_E;
		}

		if (frame < start || frame >= end)
			frame = start;
		else if (frame <= end + 1)
			frame++;

		if (shooting && frame == SHOOT_E) shooting = false;
		else if (waitToDie && frame == DIE_E) isDead = true;
		
		sprite->setTextureRect({ frame * FRAME_WIDTH, 0, FRAME_WIDTH, FRAME_HEIGHT });
	}
}

bool Player::im()
{
	using namespace ImGui;

	bool chg = false;

	Value("isFacingRight", isFacingRight);
	Value("jumping", jumping);
	Value("cx", cx);
	Value("cy", cy);

	Value("rx", rx);
	Value("ry", ry);

	sf::Vector2i pix = getPosGrid();
	chg |= DragInt2("pix x/pix y", &pix.x, 1.0f, -2000, 2000);
	if (chg)
		setPosPixel(pix.x, pix.y);

	chg |= DragInt2("cx/cy", &cx, 1.0f, -2000, 2000);

	sf::Vector2f coo = { cx + rx, cy + ry };
	bool chgCoo = DragFloat2("coo grid x/y", &coo.x, 1.0f, -2000, 2000);
	if (chgCoo)
		setPosGrid(coo.x, coo.y);

	chg |= DragFloat2("dx/dy", &dx, 0.01f, -20, 20);
	chg |= DragFloat2("frx/fry", &frx, 0.001f, 0, 1);
	chg |= DragFloat("gravy/fry", &gravity, 0.001f, -2, 2);

	if (Button("reset"))
	{
		cx = 3;
		cy = 54;
		rx = 0.5f;
		ry = 0.99f;

		dx = dy = 0;
		jumping = false;
	}

	return chg || chgCoo;
}