#pragma once
#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <unordered_map>
#include "Protocol.h"
#include "GameRoom.h"

class Server;

class GameServer {
	Server& myServer_;
	 std::unordered_map<SOCKET, Client>& clients_;
	 std::list<GameRoom> rooms_;

	void createNewRoom(SOCKET sock);   

public:

	GameServer(Server& server, std::unordered_map<SOCKET, Client>& clients);

	void processMessage(SOCKET client, const ptcl::Message& message);

	std::size_t gameRoomCount() const;
};	   

#endif //^^GAMESERVER_H