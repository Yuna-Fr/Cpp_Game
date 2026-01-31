#include "Entity.hpp"
#include "C.hpp"

Entity::Entity(std::shared_ptr<sf::Sprite> _sprite) : sprite(_sprite) {}

void Entity::dies()
{
	waitToDie = true;
}

void Entity::setPosPixel(float px, float py)
{
	cx = px / C::GRID_SIZE;
	cy = py / C::GRID_SIZE;

	rx = (px - (cx * C::GRID_SIZE)) / (float)C::GRID_SIZE;
	ry = (py - (cy * C::GRID_SIZE)) / (float)C::GRID_SIZE;

	syncPos();
}

void Entity::setPosGrid(float _cx, float _cy)
{
	cx = (int)_cx;
	rx = _cx - cx;

	cy = (int)_cy;
	ry = _cy - cy;

	syncPos();
}

void Entity::syncPos() 
{
	sf::Vector2f pos = { (cx + rx) * C::GRID_SIZE, (cy + ry) * C::GRID_SIZE };
	sprite->setPosition(pos);
}

void Entity::setGravity(bool on)
{
	if (on && gravity == 80 || !on && gravity == 0)
		return;

	gravity = on ? 80 : 0;
}

void Entity::draw(sf::RenderWindow& win)
{
	if (sprite)
		win.draw(*sprite);
}

sf::Vector2i Entity::getPosGrid()
{
	return sf::Vector2i((cx + rx), (cy + ry));
}