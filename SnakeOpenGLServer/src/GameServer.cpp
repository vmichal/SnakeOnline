#include "GameServer.h"
#include "Protocol.h"
#include <string>

GameServer::GameServer(Server& server, std::unordered_map<SOCKET, Client>& clients)
	:myServer_(server), clients_(clients) {}




void GameServer::processMessage(SOCKET client, const ptcl::Message& message) {
	switch (message.type_) {
		case ptcl::MessageType::RoomCreationRequest:
			createNewRoom(client);
			break;

	}
}

void GameServer::createNewRoom(SOCKET sock) {

	rooms_.emplace_back(&clients_[sock]);	  

}

std::size_t GameServer::gameRoomCount() const {
	return rooms_.size();
}