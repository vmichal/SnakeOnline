#include "Server.h"
#include "Log.h"
#include "Protocol.h"
#include <string>
#include <vector>
#include <numeric>
#include <cassert>
#include <chrono>


#pragma comment(lib, "ws2_32.lib")

namespace {
	std::vector<SOCKET> getPendingSockets(const std::vector<SOCKET>& allSockets) {
		std::vector<SOCKET> active;
		active.reserve(allSockets.size());
		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 1;
		fd_set fdSet;
		auto iter = allSockets.begin(), end = allSockets.end();
		for (; std::distance(iter, end) >= FD_SETSIZE; iter += FD_SETSIZE) {
			fdSet.fd_count = FD_SETSIZE;
			std::memcpy(fdSet.fd_array, &*iter, sizeof(fdSet.fd_array));
			if (select(0, &fdSet, nullptr, nullptr, &timeout) == SOCKET_ERROR) {
				Log::error() << "Error # " << WSAGetLastError() << " occured while selecting active sockets!";
				continue;
			}
			active.insert(active.end(), fdSet.fd_array, fdSet.fd_array + fdSet.fd_count);
		}
		if (std::ptrdiff_t dist = std::distance(iter, end)) {
			fdSet.fd_count = dist;
			std::memcpy(fdSet.fd_array, &*iter, sizeof(SOCKET)*dist);
			if (select(0, &fdSet, nullptr, nullptr, &timeout) == SOCKET_ERROR)
				Log::error() << "Error # " + WSAGetLastError() << " occured while selecting.";
			else
				active.insert(active.end(), std::begin(fdSet.fd_array), std::end(fdSet.fd_array));
		}
		return active;
	}

}

#pragma region NewClientListener

NewClientListener::NewClientListener(Server& s, unsigned short port)
	: myServer_(s) {
	listening_ = socket(AF_INET, SOCK_STREAM, 0);
	assert(listening_ != INVALID_SOCKET);

	Log::log() << "Created socket " << listening_ << std::endl;
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening_, (sockaddr*)&hint, sizeof(hint));
	Log::log() << "Socket " << listening_ << " bound\n";
}

NewClientListener::~NewClientListener() {
	if (running_)
		stopListening();
	if (listeningThread_.joinable())
		listeningThread_.join();
	closesocket(listening_);
}

void NewClientListener::listeningThreadLoop() {
	fd_set fd;
	timeval timeout;
	timeout.tv_usec = 10;
	while (running_) {
		fd.fd_count = 1;
		fd.fd_array[0] = listening_;
		if (select(0, &fd, nullptr, nullptr, &timeout) == SOCKET_ERROR) {
			Log::error() << "Error polling listening socket. Err # " << WSAGetLastError();
			continue;
		}
		if (running_ && fd.fd_count)
			acceptNewClient();
	}
}

void NewClientListener::startListening() {
	listen(listening_, SOMAXCONN);
	running_ = true;
	listeningThread_ = std::thread(&NewClientListener::listeningThreadLoop, this);
	Log::log() << "Server listening for new ppl";
}

void NewClientListener::stopListening() {
	running_ = false;
	Log::log() << "Server stops listening for new guys";
}

void NewClientListener::acceptNewClient() {
	sockaddr_in client;
	int clientSize = sizeof(client);
	SOCKET clientSocket = accept(listening_, (sockaddr*)&client, &clientSize);
	if (clientSocket == INVALID_SOCKET) {
		Log::error("Error estabilishing connection");
		return;
	}
	char hostName[NI_MAXHOST];
	char service[NI_MAXSERV];
	char ipBuffer[32];
	ZeroMemory(hostName, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);
	ZeroMemory(ipBuffer, 32);

	inet_ntop(AF_INET, &client.sin_addr, ipBuffer, 32);
	if (!getnameinfo((sockaddr*)&client, clientSize, hostName, NI_MAXHOST, service, NI_MAXSERV, 0))
		Log::log() << "Client " << hostName << " [" << ipBuffer << "] connected on port " << service << '\n';
	else {
		std::memcpy(hostName, ipBuffer, sizeof(ipBuffer));
		Log::log() << "DNS lookup failed, host " << hostName << "connected on port" << service << '\n';
	}
	myServer_.addClient(clientSocket, hostName, service, ipBuffer);
}
#pragma endregion

Server::Server(unsigned short port)
	:gameServer_(*this, clients_), newClientListener_(*this, port) {
}

Server::~Server() {
	running_ = false;
	if (clientProcessingThread_.joinable())
		clientProcessingThread_.join();
}

void Server::threadLoop() {
	while (running_)
		for (auto sock : getPendingSockets(clientSockets_))
			processClientRequest(sock);
}

void Server::quit() {
	Log::log("Closing server");
	running_ = false;
	newClientListener_.stopListening();
	if (clientProcessingThread_.joinable())
		clientProcessingThread_.join();
	std::lock_guard<std::mutex> lock(clientsMutex_);
	for (auto sock : clientSockets_)
		removeClient(sock);
	Log::log("All connections closed");
}

void Server::listClients() const {
	Log::log("List of connected clients:");
	std::lock_guard<std::mutex> guard(clientsMutex_);
	int counter = 0;
	for (auto sock : clientSockets_) {
		const auto& data = clients_.at(sock);
		Log::log() << ++counter << ":\t" << data.domainName_ << " [" << data.ip_ << ':' << data.service_ << "]\t"
			<< data.players_.size() << " player(s)\n" << data.currentState_;
	}
}

int Server::run() {
	Log::log("Starting server");
	running_ = true;
	newClientListener_.startListening();
	clientProcessingThread_ = std::thread(&Server::threadLoop, this);

	while (running_) {
		std::string input = Log::input().getline();
		if (running_ && input.size())
			processLocalCommand(input);
	}
	return 0;
}

void Server::processLocalCommand(std::string_view command) {
	if (!command.compare("exit") || !command.compare("quit"))
		return quit();
	if (!command.compare("list"))
		return listClients();
	Log::warning() << "Unknown command " << command.data() << std::endl;
}

void Server::removeClient(SOCKET s) {
	auto &data = clients_[s];
	Log::log() << "Closing connection with " << data.domainName_ << " [" << data.ip_ << "], connected on port " << data.service_ << std::endl;
	closesocket(s);

	std::lock_guard<std::mutex> lock(clientsMutex_);
	clients_.erase(s);
	*std::find(clientSockets_.begin(), clientSockets_.end(), s) = clientSockets_.back();  //remove socket by replacing it with the last one
	clientSockets_.pop_back();
}

void Server::sendInfoForNewClient(SOCKET client) {
	ptcl::Message msg(ptcl::MessageType::InfoForNewClientResponse, sizeof(ptcl::InfoForNewClient));
	ptcl::InfoForNewClient * info = std::launder(reinterpret_cast<ptcl::InfoForNewClient*>(msg.data_.data()));
	info->gameRoomsCount_ = gameServer_.gameRoomCount();
	info->clientsOnline_ = clientSockets_.size();
	{
		std::lock_guard<std::mutex> lock(clientsMutex_);
		clients_[client].currentState_ = ClientState::GlobalChat;
		info->playersCount_ = std::accumulate(clients_.begin(), clients_.end(), 0u,
			[](std::size_t val, decltype(*clients_.begin()) pair) -> std::size_t {
			return val + pair.second.players_.size();
		});
	}
	std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	info->serverTime_ = *std::localtime(&time);
	ptcl::sendThroughNet(msg, client);
}

void Server::broadcastGlobalMessage(ptcl::Message& msg) const {
	Log::log() << "Broadcast:\n" << msg.data_.data();
	for (auto sock : clientSockets_)
		ptcl::sendThroughNet(msg, sock);
}

void Server::processClientRequest(SOCKET client) {
	if (ptcl::clientDisconnected(client))
		return removeClient(client);

	ptcl::Message msg = ptcl::receiveFromNet(client);
	assert(msg.type_ != ptcl::MessageType::InfoForNewClientResponse);
	assert(msg.type_ != ptcl::MessageType::Invalid);

	switch (msg.type_) {
		case ptcl::MessageType::InfoForNewClientRequest:
			return sendInfoForNewClient(client);
		case ptcl::MessageType::PingRequest:
			return ptcl::pingRespond(client);
		case ptcl::MessageType::GlobalChat:
			return broadcastGlobalMessage(msg);
	}

	gameServer_.processMessage(client, msg);
}


