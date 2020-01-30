#include "Game.h"
#include "Application.h"
#include "GL\glew.h"
#include "Log.h"
#include <random>

namespace snake {

	Game::Game(Application& app, std::size_t playerCount, Size size, std::vector<std::pair<std::string, Player::Controls>>& players)
		: myApplication_(app), size_(size) {
		unsigned polickaCount = size_.width_ * size_.height_;
		policka_.reserve(polickaCount);
		for (unsigned y = 0; y < size_.height_; ++y)
			for (unsigned x = 0; x < size_.width_; ++x)
				policka_.emplace_back(x, y);
		app.graphics().resize(size_.width_ + 1, size_.height_ + 1);

		players_.reserve(players.size());
		for (const auto&[playerName, controls] : players)
			players_.emplace_back(*this, playerName, controls);
	}

	void Game::start() {
		std::mt19937 gen((std::random_device()()));
		std::uniform_int_distribution<unsigned> distribX(0, size_.width_ - 1);
		std::uniform_int_distribution<unsigned> distribY(0, size_.height_ - 1);
		for (Player& player : players_) {
			unsigned x, y;
			do {
				x = distribX(gen);
				y = distribY(gen);
			} while (policka_[y*size_.width_ + x].snakes_.size());
			player.getSnake().setStaringLocation(policka_[y*size_.width_ + x]);
		}
		running_ = true;
		thread_ = std::thread(&Game::gameLoop, this);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		for (int i = 3; i; i++) {
			std::cout << i << '\n';
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		std::cout << "Game is running\n";
	}

	void Game::draw() const {
		MainGraphics& graphics = myApplication_.graphics();
		std::vector<const Pole*> squares;
		squares.reserve(size_.width_ * size_.height_);
		squares.push_back(food_);
		for (const Pole& pole : policka_)
			if (pole.snakes_.size())
				squares.push_back(&pole);
		std::vector<unsigned> indices;
		indices.reserve(squares.size() << 2);
		auto inserter = std::back_inserter(indices);
		for (const auto& pole : squares) {
			const unsigned x = pole->x_, y = pole->y_;
			inserter = (size_.width_ + 1)*y + x;		 //top left
			inserter = (size_.width_ + 1)*y + x + 1;	 //top right
			inserter = (size_.width_ + 1)*(y + 1) + x;	 //bottom left

			inserter = (size_.width_ + 1)*y + x + 1;	 //top right
			inserter = (size_.width_ + 1)*(y + 1) + x;	   //bottom left
			inserter = (size_.width_ + 1)*(y + 1) + x + 1; //bottom right
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned)*indices.size(), indices.data(), GL_STATIC_DRAW);
		graphics.draw(indices.size());
	}

	void Game::move() {
		for (Player& player : players_)
			player.move();
	}

	void Game::generateNewFood() {
		static std::mt19937 twister((std::random_device()()));
		unsigned x, y;
		do {
			do {
				x = twister();
			} while (x < size_.width_);
			do {
				y = twister();
			} while (y < size_.height_);
		} while (policka_[y * size_.width_ + x].snakes_.size());
		if (food_)
			food_->isFood_ = false;
		food_ = &policka_[y * size_.width_ + x];
		food_->isFood_ = true;
	}

	void Game::checkCollisions() {
		if (auto& snks = food_->snakes_; snks.size()) {
			for (Snake* snk : snks)
				snk->getPlayer().increaseScore();	
			generateNewFood();
		}
		if (!checkCollisions_)														   
			return;			
		for (Player & player : players_) 
			if (std::vector<Snake*> snakesOnPole = player.snake_.getHead().snakes_;  snakesOnPole.size() > 1) {
				snakesOnPole.erase(std::find(snakesOnPole.begin(), snakesOnPole.end(), &player.snake_));
				paused_ = true;
				auto iter = snakesOnPole.begin();
				std::cout << "Player " << player.name_ << " has bumped into " << (*iter)->getPlayer().name_;
				for (++iter; iter != snakesOnPole.end(); ++iter)
					std::cout << ", " << (*iter)->getPlayer().name_;	
			}
	}																				   

	void Game::gameLoop() {
		std::chrono::time_point<std::chrono::steady_clock> nextFrame;
		while (running_) {
			nextFrame = std::chrono::steady_clock::now() + std::chrono::nanoseconds(950'000'000 / frameRate_); //odborný odhad
			draw();																			  //skoro jedna sekunda dìlená frameratem
			if (!paused_) {																	  
				move();
				checkCollisions();
			}
			std::this_thread::sleep_until(nextFrame);
		}
	}

	void Game::endGame() {								   
		running_ = false;
		thread_.join();
	}

	void Game::togglePause(bool yes) {
		paused_ = yes;
		std::cout << (yes ? "Game paused\n" : "Game resumed\n");
	}
	
	void Game::togglePause() {
		togglePause(!paused_);
	}

	void Game::toggleCollisions(bool yes) {
		checkCollisions_ = yes;
		std::cout << (yes ? "Collisions turned on\n" : "Collisions turned off\n");
	}

	void Game::toggleCollisions() {
		toggleCollisions(!checkCollisions_);
	}

	void Game::keyCallback(int key, int scanCode, int action, int mods) {
		if (action == GLFW_PRESS) {
			switch (key) {
				case GLFW_KEY_ESCAPE:
					endGame();
					break;
				case GLFW_KEY_KP_ADD:
					frameRate_++;
					break;
				case GLFW_KEY_KP_SUBTRACT:
					frameRate_--;
					break;
				case GLFW_KEY_SPACE:
					togglePause();
					break;
				case GLFW_KEY_TAB:
					toggleCollisions();
					break;
				default:
					for (auto& player : players_)
						if (player.tryKeyCallback(key))
							break;
					std::cout << "No key set.\n";
			}
		}
	}
}