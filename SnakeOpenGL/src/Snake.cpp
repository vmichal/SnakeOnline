#include "Snake.h"
#include <vector>
#include "Game.h"
#include <cassert>
#include <algorithm>

namespace snake {

	Snake::Snake(Player& player)
		:myPlayer_(player) 
	{	}

	void Snake::turn(Direction direction) {
		switch (direction) {
			case Direction::up:
				if (lastMove_ == Direction::down)
					return;
				break;
			case Direction::down:
				if (lastMove_ == Direction::up)
					return;
				break;
			case Direction::left:
				if (lastMove_ == Direction::right)
					return;
				break;
			case Direction::right:
				if (lastMove_ == Direction::left)
					return;
		}
		nextMove_ = direction;
	}

	void Snake::move() {
		auto& snakes = clanky_.back()->snakes_;	   //remove last policko
		clanky_.pop_back();
		auto iter = std::find_if(snakes.begin(), snakes.end(), [this](decltype(*snakes.begin()) ptr) -> bool {
			return ptr == this;
		});
		assert(iter != snakes.end());
		snakes.erase(iter);
		//get new head					
		Pole * newHead = getHead().neighbours_[nextMove_];
		clanky_.push_front(newHead);
		newHead->snakes_.push_back(this);
		/*Pole * newHead = getHead().neighbours_[nextMove_];

		if (!newHead) {   //The snake had hit a wall
			collisionInfo_.state_ = CollisionInfo::Wall;
			collisionInfo_.otherPlayers_.clear();
		}
		else {
			if (newHead->snakes_.size()) {
				collisionInfo_.state_ = CollisionInfo::OtherPlayer;
				collisionInfo_.otherPlayers_.clear();
				collisionInfo_.otherPlayers_.assign(newHead->snakes_.begin(), newHead->snakes_.end());
				std::for_each(collisionInfo_.otherPlayers_->snakes_.begin(), collisionInfo_.otherPlayers_->snakes_.end(),
					[ins = std::back_inserter(collisionInfo_.otherPlayers_)](decltype(*newHead->snakes_.begin()) pair) {
					if (!pair.second)	 //pokud jsou tu ostatní 
						ins = pair.first;
				});
				collisionInfo_.otherPlayers_ = &newHead->snakes_.back().first->myPlayer_;
				if (collisionInfo_.otherPlayers_->getSnake().getHead() == *newHead) {
					collisionInfo_.state_ = CollisionInfo::OtherPlayer;
					collisionInfo_.otherPlayers_ = &newHead->snakes_.back().first->myPlayer_;
				}
			}
			clanky_.push_front(newHead);
			newHead->snakes_.emplace_back(this, 0);
		}  */
	}

	void Snake::setStaringLocation(Pole& startingPos) {
		clanky_.assign(startingSnakeLenght, &startingPos);
		assert(startingPos.snakes_.empty());			  //For the lulz
		startingPos.snakes_.reserve(startingSnakeLenght);
		for (int i = startingSnakeLenght; i ; --i)
			startingPos.snakes_.emplace_back(this);
	}
}