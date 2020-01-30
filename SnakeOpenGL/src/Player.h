#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#include "Snake.h"
#include <string>

namespace snake {

	class Game;

	class Player {

		friend class Game;
	public:
		struct Controls {
			int up_ = 0, down_ = 0, left_ = 0, right_ = 0;
		};
	private:
		Game & myGame_;
		Snake snake_;
		unsigned score_ = 0;
		std::string name_;				  
		Controls controls_;

	public:
		Player(Game& game, const std::string& name, const Controls& controls);
		bool tryKeyCallback(int key);

		void increaseScore();

		void move();

		Snake& getSnake() { return snake_; }
	};

}

#endif // ^^PLAYER_H