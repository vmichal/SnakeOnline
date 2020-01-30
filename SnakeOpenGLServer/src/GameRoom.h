#pragma once

#ifndef GAMEROOM_H
#define GAMEROOM_H
#include <vector>
#include <string>
#include "Client.h"

class GameRoom {
	std::vector<Client*> clients_;

public:
	GameRoom(Client *owner) {
		clients_.push_back(owner);
	}

};


#endif // ^^ GAMEROOM_H
