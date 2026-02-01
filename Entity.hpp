#pragma once

#include "SFML/Graphics.hpp"
#include "Game.hpp"

namespace sf { class RenderWindow; }

class Entity
{
	public:
		Game&	g = *Game::instance;

		std::shared_ptr<sf::Sprite> sprite = 0;

		// Base coordinates
		int		cx = 0;
		int		cy = 0;
		float	rx = 0;
		float	ry = 0;

		// Movements
		float	dx = 0.f;
		float	dy = 0.f;
		float	gravity = 80.f;

		bool	isFacingRight = true;
		bool	isDead = false;

		bool	waitToDie = false;
		int		frame = 0;
		float	animTimer = 0.f;
		float	animSpeed = 0.1f;

	public:
		Entity(std::shared_ptr<sf::Sprite> sprite);

		virtual void	update(double dt) = 0;
		virtual bool	im() = 0;
		virtual void	dies();

		void			setPosPixel(float px, float py);
		void			setPosGrid(float cx, float cy);
		void			syncPos();
		void			setGravity(bool on);
		void			draw(sf::RenderWindow& win);

		sf::Vector2i	getPosGrid();
};