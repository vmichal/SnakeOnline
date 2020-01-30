#pragma once
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <WS2tcpip.h>
#include <vector>
#include <string>
#include <cassert>
#include <chrono>

constexpr int MAX_PLAYERS_ONE_TASTATUR = 4;

namespace ptcl {

	enum class MessageType : unsigned {
		Invalid,
		InfoForNewClientRequest = 1,
		InfoForNewClientResponse,

		PingRequest,
		PingResponse,

		EndConnection,

		GlobalChat,

		RoomCreationRequest,
		RoomCreationData,
		RoomCreationCancel
	};

	struct Message {
		MessageType type_;
		std::size_t size_;
		std::vector<char> data_;

		Message(MessageType type, std::size_t size)
			: type_(type), size_(size), data_(size) {}
	};

	struct InfoForNewClient {
		unsigned clientsOnline_, gameRoomsCount_, playersCount_;
		std::tm serverTime_;
	};

	struct RoomCreationData {
		std::size_t playerLimit_, nameLength_, passwordLength_;
		//starting at offset 12, nameLength_ chars are name of room, then \0 and then passwordLenght_ chars of password
	};

	bool clientDisconnected(SOCKET s);

	void sendThroughNet(Message& message, SOCKET sock);

	Message receiveFromNet(SOCKET sock);

	std::chrono::milliseconds pingMeasure(SOCKET client);

	void pingRespond(SOCKET sock);
}


#endif //^^ PROTOCOL_H