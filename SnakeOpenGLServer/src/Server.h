#pragma once
#ifndef SERVER_H
#define SERVER_H

#include <WS2tcpip.h>
#include <thread>
#include <map>
#include <string_view>
#include <mutex>

#include "Client.h"
#include "GameServer.h"

class Server;

class NewClientListener {

private:
	Server & myServer_;
	SOCKET listening_ = INVALID_SOCKET;
	volatile bool running_ = false;
	std::thread listeningThread_;

	void listeningThreadLoop();

	void acceptNewClient();

public:
	NewClientListener(Server &s, unsigned short port);
	~NewClientListener();

	void startListening();
	void stopListening();

};

class Server {
private:
	std::unordered_map<SOCKET, Client> clients_;
	GameServer gameServer_;
	NewClientListener newClientListener_;
	mutable std::mutex clientsMutex_;
	std::vector<SOCKET> clientSockets_;
	volatile bool running_ = false;
	std::thread clientProcessingThread_;

	void removeClient(SOCKET s);

	void processClientRequest(SOCKET sock);


	void processLocalCommand(std::string_view command);

	void threadLoop();

	void sendInfoForNewClient(SOCKET s);

	void broadcastGlobalMessage(ptcl::Message& msg) const;

	//Internal commands
	void quit();
	void listClients() const;
public:

	Server(unsigned short port);
	~Server();

	int run();

	template<typename T, typename U, typename V>
	void addClient(SOCKET clientSocket, T &&name, U&&service, V&&ip) {
		std::lock_guard<std::mutex> lock(clientsMutex_);
		if (!running_) {
			closesocket(clientSocket);
			return;
		}
		clients_[clientSocket] = Client(name, service, ip);
		clientSockets_.push_back(clientSocket);
	}
};




#endif //^^SERVER_H