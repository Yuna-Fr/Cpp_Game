#include <imgui.h>
#include <array>
#include <vector>
#include <fstream>

#include "C.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include "Pet.hpp"
#include "Enemy.hpp"
#include "Bullet.hpp"
#include "PetBullet.hpp"

#include "HotReloadShader.hpp"

Game* Game::instance = 0;

static int cols = C::RES_X / C::GRID_SIZE;
static int lastLine = C::RES_Y / C::GRID_SIZE - 1;

Game::Game(sf::RenderWindow * win) {
	this->win = win;
	instance = this;
	camera = win->getDefaultView();
	
	bg = sf::RectangleShape(Vector2f((float)win->getSize().x, (float)win->getSize().y));

	bool isOk = bgTexture.loadFromFile("res/underwater_background.png");
	if (!isOk) printf("ERR : LOAD FAILED\n");
	else bgTexture.setSmooth(false);

	bg.setTexture(&bgTexture);
	bg.setSize(sf::Vector2f(C::RES_X, C::RES_Y));

	bgShader = new HotReloadShader("res/bg.vert", "res/bg.frag"); 

	// Set Textures
	playerTexture.loadFromFile("res/fish.png");
	playerTexture.setSmooth(false);
	petTexture.loadFromFile("res/pet.png");
	petTexture.setSmooth(false);
	enemyTexture.loadFromFile("res/enemy_run.png");
	enemyTexture.setSmooth(false);
	bulletTexture.loadFromFile("res/bullet.png");
	bulletTexture.setSmooth(false);
	petBulletTexture.loadFromFile("res/petBullet.png");
	petBulletTexture.setSmooth(false);

	loadLevel("levelSave.txt");
}

#pragma region Spawning

void Game::cacheWalls()
{
	wallSprites.clear();
	for (Vector2i& w : walls)
	{
		sf::RectangleShape rect(Vector2f(16, 16));
		rect.setPosition((float)w.x * C::GRID_SIZE, (float)w.y * C::GRID_SIZE);
		rect.setFillColor(sf::Color(5, 12, 28));
		wallSprites.push_back(rect);
	}
}

void Game::spawnPlayer(float x, float y)
{
	auto sprite = std::make_shared<sf::Sprite>(playerTexture);
	player = std::make_shared<Player>(std::move(sprite), x, y);

	auto Petsprite = std::make_shared<sf::Sprite>(petTexture);
	pet = std::make_shared<Pet>(Petsprite);

	printf("Player initialized");
}

void Game::spawnEnemies(float x, float y)
{
	auto sprite = std::make_shared<sf::Sprite>(enemyTexture);
	enemies.push_back(std::make_shared<Enemy>(sprite, x, y));
}

void Game::spawnPlayerBullet(float x, float y, bool faceRight)
{
	auto sprite = std::make_shared<sf::Sprite>(bulletTexture);
	bullets.push_back(std::make_shared<Bullet>(sprite, x, y, faceRight));
}

std::shared_ptr<PetBullet> Game::spawnPetBullet(float x, float y, const std::shared_ptr<Enemy>& e)
{
	auto sprite = std::make_shared<sf::Sprite>(petBulletTexture);
	auto pbullet = std::make_shared<PetBullet>(sprite, x, y, e);
	bullets.push_back(pbullet);
	return pbullet;
}

#pragma endregion

void Game::processInput(sf::Event ev) {
	if (ev.type == sf::Event::Closed){
		win->close();
		closing = true;
		return;
	}
}

static double g_time = 0.0;
static double g_tickTimer = 0.0;

static sf::VertexArray va;
static RenderStates vaRs;
static std::vector<sf::RectangleShape> rects;

void Game::update(double dt)
{
	if (inEditor) return;

	dt = std::min(dt, 1.0 / 30.0); //cap to avoid dt stacking when exiting the window

	pollInput(dt);

	g_time += dt;
	if (bgShader) bgShader->update(dt);

	updateShake(dt);

	player->update(dt);
	pet->update(dt);

	for (auto& e : enemies)
		e->update(dt);

	for (auto& b : bullets)
		b->update(dt);
	
	manageDeadEntities();
}

void Game::draw(sf::RenderWindow& win) {
	if (closing) return;

	win.setView(camera);

	sf::RenderStates states = sf::RenderStates::Default;
	sf::Shader* sh = &bgShader->sh;
	states.blendMode = sf::BlendAdd;
	states.shader = sh;
	states.texture = &bgTexture;
	sh->setUniform("texture", bgTexture);
	//sh->setUniform("time", g_time);
	win.draw(bg, states);

	if (inEditor) drawGrid(win);

	for (sf::RectangleShape& r : wallSprites)
		win.draw(r);

	for (sf::RectangleShape& r : rects)
		win.draw(r);

	player->draw(win);
	pet->draw(win);

	for (auto& e : enemies)
		e->draw(win);

	for (auto& b : bullets)
		b->draw(win);
}

void Game::pollInput(double dt) 
{
	if (player == nullptr || player->waitToDie) return;

	player->inputX = 0;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) player->inputX = -10;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) player->inputX = +10;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
	{
		player->shoot();
		shakeScreen(0.1f, 3.f);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && !player->jumping)  player->jump();

	if (sf::Joystick::isConnected(0)) // If gamepad connected
	{
		float lateralSpeed = 4.0f;
		float deadzone = 15.f;
		float xAxis = sf::Joystick::getAxisPosition(0, sf::Joystick::X);

		if (xAxis < -deadzone) player->inputX -= lateralSpeed;
		if (xAxis > deadzone)  player->inputX += lateralSpeed;

		if (sf::Joystick::isButtonPressed(0, 0) && !player->jumping) player->jump();
		if (sf::Joystick::isButtonPressed(0, 1)) 
		{
			player->shoot();
			shakeScreen(0.1f, 3.f);
		}
	}
}

bool Game::hasCollision(int gridx, int gridy)
{
	if (gridx < 1) 
		return true;

	auto wallRightX = (C::RES_X / C::GRID_SIZE) - 1;
	if (gridx >= wallRightX) 
		return true;

	for (Vector2i& w : walls) 
	{
		if ((w.x == gridx) && (w.y == gridy)) 
			return true;
	}

	return false;
}

void Game::manageDeadEntities()
{
	// Detect bullet collisions with walls

	if (enemies.empty() && !bullets.empty())
	{
		for (auto bIndex = bullets.begin(); bIndex != bullets.end(); )
		{
			if ((*bIndex)->isDead)
			{
				bIndex = bullets.erase(bIndex);
				continue;
			}

			++bIndex;
		}
	}
	else
	{
		for (auto eIndex = enemies.begin(); eIndex != enemies.end(); )
		{
			auto& e = *eIndex;

			if (e->isDead)
			{
				eIndex = enemies.erase(eIndex);
				continue;
			}

			// Detect bullets collisions with entities
			for (auto bIndex = bullets.begin(); bIndex != bullets.end(); )
			{
				auto& b = *bIndex;

				if (b->isDead)
				{
					bIndex = bullets.erase(bIndex);
					continue;
				}

				if (e->cx == b->cx && e->cy == b->cy)
				{
					e->dies();
					b->dies();
				}

				++bIndex;
			}

			// Detect player collision with entities
			if (!player->isDead && e->cx == player->cx && e->cy == player->cy)
				player->dies();

			++eIndex;
		}
	}
}

void Game::shakeScreen(float duration, float strength)
{
	shakeDuration = duration;
	shakeTime = duration;
	shakeStrength = strength;
}

void Game::updateShake(double dt)
{
	if (shakeTime > 0.f)
	{
		shakeTime -= (float)dt;

		float strength = shakeStrength * (shakeTime / shakeDuration);

		float offsetX = ((rand() / (float)RAND_MAX) * 2.f - 1.f) * strength;
		float offsetY = ((rand() / (float)RAND_MAX) * 2.f - 1.f) * strength;

		camera.setCenter(
			C::RES_X * 0.5f + offsetX,
			C::RES_Y * 0.5f + offsetY
		);
	}
	else if (camera.getCenter().x != C::RES_X * 0.5f || camera.getCenter().y != C::RES_Y * 0.5f)
	{
		camera.setCenter(C::RES_X * 0.5f, C::RES_Y * 0.5f);
	}
}

int blendModeIndex(sf::BlendMode bm) {
	if (bm == sf::BlendAlpha) return 0;
	if (bm == sf::BlendAdd) return 1;
	if (bm == sf::BlendNone) return 2;
	if (bm == sf::BlendMultiply) return 3;
	return 4;
};

#pragma region Level Editor

void Game::drawGrid(sf::RenderWindow& win)
{
	static sf::VertexArray gridLines(sf::Lines);

	if (gridLines.getVertexCount() == 0)
	{
		int cols = C::RES_X / C::GRID_SIZE;
		int rows = C::RES_Y / C::GRID_SIZE;

		sf::Color lineColor(150, 150, 150, 50);

		// Vertical lines
		for (int i = 0; i <= cols; ++i)
		{
			float x = i * C::GRID_SIZE;
			gridLines.append(sf::Vertex({ x, 0 }, lineColor));
			gridLines.append(sf::Vertex({ x, (float)C::RES_Y }, lineColor));
		}

		// Horizontal lines
		for (int j = 0; j <= rows; ++j)
		{
			float y = j * C::GRID_SIZE;
			gridLines.append(sf::Vertex({ 0, y }, lineColor));
			gridLines.append(sf::Vertex({ (float)C::RES_X, y }, lineColor));
		}
	}

	win.draw(gridLines);
}

void Game::loadLevel(const std::string& filename)
{
	std::ifstream file(filename);

	if (!file.is_open()) return;

	player.reset();
	enemies.clear();
	bullets.clear();
	walls.clear();

	std::string type;
	int x, y;

	while (file >> type >> x >> y)
	{
		if (type == "WALL")
		{
			walls.emplace_back(x, y);
		}
		else if (type == "ENEMY")
		{
			spawnEnemies((float)x, (float)y);
		}
		else if (type == "PLAYER")
		{
			spawnPlayer((float)x, (float)y);
		}
	}

	cacheWalls();
	file.close();
}

void Game::saveLevel(const std::string& filename)
{
	std::ofstream file(filename);

	if (!file.is_open()) return;
	 
	if (player)
		file << "PLAYER " << player->cx << " " << player->cy << "\n";

	for (auto e : enemies)
		file << "ENEMY " << e->cx << " " << e->cy << "\n";

	for (auto& w : walls)
		file << "WALL " << w.x << " " << w.y << "\n";

	file.close();
}

#pragma endregion

void Game::im()
{
	using namespace ImGui;

	if (!inEditor) 
	{
		if (Button("Edit", ImVec2(320.f, 30.f)))
			inEditor = true;

		if (TreeNodeEx("Walls", 0))
		{
			for (auto& w : walls)
			{
				Value("x", w.x);
				Value("y", w.y);
			}
			TreePop();
		}

		for (size_t i = 0; i < enemies.size(); ++i)
		{
			if (TreeNodeEx(("Enemy " + to_string(i)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				enemies[i]->im();
				TreePop();
			}
		}

		if (TreeNodeEx("Player", ImGuiTreeNodeFlags_DefaultOpen))
		{
			player->im();
			TreePop();
		}
	}
	else
	{
		if (Button("Play", ImVec2(320.f, 30.f))) 
			inEditor = false;

		if (Button("Walls")) editMode = CellType::Wall;
		if (Button("Enemies")) editMode = CellType::Enemy;
		if (Button("PlayerSpawn")) editMode = CellType::Player;

		if (Button("Save Level")) saveLevel("levelSave.txt");

		if (!GetIO().WantCaptureMouse && IsMouseClicked) // ignore click on level if clicking on debug window
		{
			sf::Vector2i mousePos = GetMousePos();
			int gridX = std::floor(mousePos.x / C::GRID_SIZE);
			int gridY = std::floor(mousePos.y / C::GRID_SIZE);

			if (IsMouseDown(ImGuiMouseButton_Left)) 
			{
				if (editMode == CellType::Wall)
				{
					for (auto& w : walls) {
						if (w.x == gridX && w.y == gridY)
							return;
					}

					walls.push_back(sf::Vector2i(gridX, gridY));
					cacheWalls();
				}
				else if (editMode == CellType::Enemy)
				{
					for (auto& e : enemies) {
						if (e && std::floor((e->cx + e->rx)) == gridX && std::floor((e->cy + e->ry)) == gridY)
							return;
					}

					spawnEnemies((float)gridX, (float)gridY);
				}
				else if (editMode == CellType::Player)
				{
					if (player)
					{
						player->setPosGrid((float)gridX, (float)gridY);
						player->ry = 0.99f;
						player->syncPos();
					}
					else
						spawnPlayer((float)gridX, (float)gridY);
				}
			}
			else if (IsMouseDown(ImGuiMouseButton_Right))
			{
				if (editMode == CellType::Wall)
				{
					for (auto& w : walls)
					{
						if (w.x == gridX && w.y == gridY)
						{
							walls.erase(std::remove(walls.begin(), walls.end(), w), walls.end());
							cacheWalls();
							break;
						}
					}
				}
				else if (editMode == CellType::Enemy)
				{
					for (auto& e : enemies)
					{
						if (e && std::floor((e->cx + e->rx)) == gridX && std::floor((e->cy + e->ry)) == gridY) {
							enemies.erase(std::remove(enemies.begin(), enemies.end(), e), enemies.end());
							break;
						}
					}
				}
				else if (editMode == CellType::Player)
				{
					if (player)
					{
						player->setPosGrid((float)gridX, (float)gridY);
						player->ry = 0.99f;
						player->syncPos();
					}
				}
			}
		}
	}
}