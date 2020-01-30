#pragma once
#ifndef POLE_H
#define POLE_H


#include <vector>
#include <map>
#include "Snake.h"

namespace snake {

	/*

	struct Vec2 {
		int  x_, y_;

		bool opacny(const Vec2& rhs) const {
			return x_ == -rhs.x_ || y_ == -rhs.y_;
		}
	};*/

	struct Pole {
		int x_, y_;					
		bool isFood_ = false;
		std::vector<std::pair<Snake*, unsigned>> snakes_;
		std::map<Direction, Pole*> neighbours_;

		Pole(unsigned x, unsigned y) : x_(x), y_(y) {}

		/*Vec2 operator-(const Pole& rhs) const {
			Vec2 res;
			res.x_ = x_ - rhs.x_;
			res.y_ = y_ - rhs.y_;
			return res;
		} */

	};

}

#endif // ^^ POLE_H