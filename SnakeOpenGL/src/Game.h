#pragma once
#ifndef GAME_H
#define GAME_H

#include "Pole.h"
#include "Player.h"
#include <vector>
#include <string>
#include <thread>
#include <chrono>

namespace snake {

	class Application;

	class Game {
	public:
		struct Size { unsigned width_, height_; };

	private:
		Application & myApplication_;
		volatile bool running_ = false,
			paused_ = true,
			checkCollisions_ = false;
		unsigned frameRate_ = 8;
		Pole * food_ = nullptr;
		Size size_;
		std::vector<Pole> policka_;	 //vector políèek, rows are contiguous
		std::vector<Player> players_;
		std::thread thread_;

		void gameLoop();
		void draw() const;
		void move();
		void checkCollisions();
		void generateNewFood();

	public:
		Game(Application& app, std::size_t playerCount, Size size, std::vector<std::pair<std::string, Player::Controls>>& players);
		void start();

		void keyCallback(int key, int scanCode, int action, int mods);

		void endGame();
		void togglePause(bool yes);
		void togglePause();
		void toggleCollisions(bool yes);
		void toggleCollisions();

		const Size& size() { return size_; }
	};
}

#endif //^^ GAME_H