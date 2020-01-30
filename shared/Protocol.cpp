#include "Protocol.h"

namespace ptcl {

	namespace {

		static std::chrono::time_point<std::chrono::system_clock> pingTimeSent;

	}

	bool clientDisconnected(SOCKET s) {
		char buffer[1];
		int bytes = recv(s, buffer, 1, MSG_PEEK);
		if (bytes < 0)
			throw std::runtime_error("An error occured while checking client connection status. Err #" + std::to_string(WSAGetLastError()));
		return bytes == 0;	 //if bytes == 0 then client has disconnected
	}

	void sendThroughNet(Message& message, SOCKET sock) {
		assert(message.size_);
		int sendResult = send(sock, reinterpret_cast<char *>(&message), offsetof(Message, data_), 0);
		if (sendResult == SOCKET_ERROR)
			throw std::runtime_error("An error occured while sending size of data. Err #" + std::to_string(WSAGetLastError()));
		sendResult = send(sock, message.data_.data(), message.size_, 0);
		if (sendResult == SOCKET_ERROR)
			throw std::runtime_error("An error occured while sending data. Err #" + std::to_string(WSAGetLastError()));
	}

	Message receiveFromNet(SOCKET sock) {
		Message message(MessageType::Invalid, 0);
		int bytes = recv(sock, reinterpret_cast<char*>(&message), offsetof(Message, data_), 0);
		assert(message.type_ != MessageType::Invalid && message.size_);
		if (bytes <= 0)
			throw std::runtime_error("An error occured while receiving. Err #" + std::to_string(WSAGetLastError()));
		message.data_.resize(message.size_);
		bytes = recv(sock, message.data_.data(), message.size_, 0);
		if (bytes <= 0)
			throw std::runtime_error("Error receiving data. Err #" + std::to_string(WSAGetLastError()));
		return message;
	}

	std::chrono::milliseconds pingMeasure(SOCKET client) {
		Message pingMsg(MessageType::PingRequest, 0);
		pingTimeSent = std::chrono::system_clock::now();	 //TODO, nebude fungovat, ale kód je dobrej
		sendThroughNet(pingMsg, client);
		Message msg = receiveFromNet(client);
		auto timeReceived = std::chrono::system_clock::now();
		assert(msg.type_ == MessageType::PingResponse);
		return std::chrono::duration_cast<std::chrono::milliseconds>(pingTimeSent - timeReceived);
	}

	void pingRespond(SOCKET sock) {
		Message pingResponse(MessageType::PingResponse, 0);
		sendThroughNet(pingResponse, sock);
	}			   									   
}