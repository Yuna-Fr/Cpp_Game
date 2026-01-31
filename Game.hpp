#pragma once

#include <vector>

#include "SFML/Window.hpp"
#include "sys.hpp"
#include "SFML/Graphics.hpp"

#include "SFML/System.hpp"

using namespace sf;

class Player;
class Pet;
class PetBullet;
class Enemy;
class Entity;
class HotReloadShader;

class Game
{
public:
	enum class CellType { Wall, Enemy, Player };

	sf::RenderWindow*				win = nullptr;
	sf::View						camera;

	sf::RectangleShape				bg;
	HotReloadShader*				bgShader = nullptr;

	bool							closing = false;

	//Screen shake
	float							shakeTime = 0.f;
	float							shakeDuration = 0.f;
	float							shakeStrength = 0.f;

	//Entities and Walls
	std::shared_ptr<Player>			player;
	std::shared_ptr<Pet>			pet;
	std::vector< std::shared_ptr<Enemy>> enemies;
	std::vector<std::shared_ptr<Entity>> bullets;

	std::vector<sf::Vector2i>		walls;
	std::vector<sf::RectangleShape> wallSprites;

	//Textures
	sf::Texture						bgTexture;
	sf::Texture						playerTexture;
	sf::Texture						petTexture;
	sf::Texture						enemyTexture;
	sf::Texture						bulletTexture;
	sf::Texture						petBulletTexture;

	//For Editor
	bool							inEditor = false;
	CellType						editMode = CellType::Wall;

	static Game*					instance;

	Game(sf::RenderWindow* win);

	void						cacheWalls();
	void						spawnPlayer(float x, float y);
	void						spawnEnemies(float x, float y);
	void						spawnPlayerBullet(float x, float y, bool faceRight);
	std::shared_ptr<PetBullet>	spawnPetBullet(float x, float y, const std::shared_ptr<Enemy>& e);

	void						update(double dt);
	void						draw(sf::RenderWindow& win);

	void						processInput(sf::Event ev);
	void						pollInput(double dt);

	bool						hasCollision(int gridx, int gridy);
	void						manageDeadEntities();
	void						shakeScreen(float duration, float strength);
	void						updateShake(double dt);

	void						drawGrid(sf::RenderWindow& win);
	void						loadLevel(const std::string& filename);
	void						saveLevel(const std::string& filename);

	void						im();
};