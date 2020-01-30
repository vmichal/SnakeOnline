#pragma once
#ifndef SNAKE_H
#define SNAKE_H

#include "Pole.h"
#include <deque>
#include <vector>


namespace snake {

	constexpr unsigned startingSnakeLenght = 5;

	enum class Direction {
		nowhere, up, down, left, right
	};

	class Game;
	class Player;

	class Snake {
	public:
		
	private:
		Player & myPlayer_;
		std::deque<Pole*> clanky_;
		Direction lastMove_ = Direction::nowhere,
			nextMove_ = Direction::nowhere;

	public:
		Snake(Player& player);
		void turn(Direction d);
		void move();

		void setStaringLocation(Pole& startingPos);

		Pole& getHead() const { return *clanky_.front(); }
		Player& getPlayer() const { return myPlayer_; }
	};
}


#endif // ^^ SNAKE_H