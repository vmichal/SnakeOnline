#pragma once
#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <list>
#include "Player.h"


enum class ClientState : unsigned {
	NotVerified,
	GlobalChat,
	GameRoom, 
	Playing
};

inline std::ostream& operator<<(std::ostream& stream, ClientState state) {
	switch (state) {
		case ClientState::NotVerified:
			return stream << "Not verified";
		case ClientState::GlobalChat:
			return stream << "In global chat";
		case ClientState::GameRoom:
			return stream << "In game room";
		case ClientState::Playing:
			return stream << "Playing";
	}
	return stream <<"\a\n\n Not Implemented!\n\n\a";
}

class GameRoom;

struct Client {
	ClientState currentState_ = ClientState::NotVerified;
	std::string domainName_, service_, ip_;
	std::list<Player> players_;
	GameRoom *gameRoom_;

	template<typename T, typename U, typename V>
	Client(T&& name, U&& service, V&& ip) : domainName_(name), service_(service), ip_(ip) 
	{}
	Client() = default;
};



#endif // ^^ CLIENT_H