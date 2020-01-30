#include "Player.h"
#include <iostream>

namespace snake {

	Player::Player(Game& game, const std::string& name, const Controls& controls)
		:myGame_(game), snake_(*this), name_(name), controls_(controls) {}


	bool Player::tryKeyCallback(int key) {
		if (key == controls_.up_) {
			snake_.turn(Direction::up);
			std::cout << "Key callback caught\n";
			return true;
		}
		if (key == controls_.down_) {
			snake_.turn(Direction::down);
			std::cout << "Key callback caught\n";
			return true;
		}
		if (key == controls_.left_) {
			snake_.turn(Direction::left);
			std::cout << "Key callback caught\n";
			return true;
		}
		if (key == controls_.right_) {
			snake_.turn(Direction::right);
			std::cout << "Key callback caught\n";
			return true;
		}
		return false;
	}
	void Player::increaseScore() {
		std::cout << "Player " << name_ << " has " << ++score_ << " points.\n";
	}

	void Player::move() {
		snake_.move();
	}
}