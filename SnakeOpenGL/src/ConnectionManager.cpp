#include "ConnectionManager.h"
#include "Log.h"
#include <string>
#include "Protocol.h"

namespace snake {

	namespace {

		SOCKET estabilishConnection() {
			std::string serverIP;
			unsigned short port;
			{
				auto in = Log::input();
				Log::prompt("Zadej pls IP adresu serveru ([xxx.yyy.zzz.www]):");
				in >> serverIP;
				Log::prompt("A port pls:");
				in >> port;
			}
			SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
			if (sock == INVALID_SOCKET) {
				Log::error() << "Error creating socket";
				return INVALID_SOCKET;
			}
			sockaddr_in hint;
			hint.sin_family = AF_INET;
			hint.sin_port = htons(port);
			inet_pton(AF_INET, serverIP.c_str(), &hint.sin_addr);

			if (connect(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
				Log::error() << "Cant connect to the server, shit's not working. Err # " << WSAGetLastError();
				return INVALID_SOCKET;
			}
			Log::log() << "Connected to server [" << serverIP << ':' << port << ']';
			return sock;
		}

		void requestInfoForNewClient(SOCKET serverSock) {
			ptcl::Message requestMsg(ptcl::MessageType::InfoForNewClientRequest, 0);
			ptcl::sendThroughNet(requestMsg, serverSock);
			Log::log() << "Attempting to connect";

			ptcl::Message msg = ptcl::receiveFromNet(serverSock);
			assert(msg.type_ == ptcl::MessageType::InfoForNewClient);
			Log::log() << "Response received";

			ptcl::InfoForNewClient *info = std::launder(reinterpret_cast<ptcl::InfoForNewClient*>(msg.data_.data()));
			Log::log() << "Welcome to global chat!\nThere are " << info->gameRoomsCount_ << " games being played and "
				<< info->clientsOnline_ << " connected clients with " << info->playersCount_ << " in global chat.\n";
			const std::size_t dateTimeBufferSize = 256;
			char dateTimeBuffer[dateTimeBufferSize];
			std::strftime(dateTimeBuffer, dateTimeBufferSize, "Server date: %e. %B %Y" "%n" "Server time: %H:%M:%S.", &info->serverTime_);
			Log::log() << dateTimeBuffer << info->serverTimeMilliseconds_;
		}
	}

	ConnectionManager::ConnectionManager(Application& app)
		:myApp_(app) {
	}

	ConnectionManager::~ConnectionManager() {
		disconnect();
	}

	void ConnectionManager::listeningThreadLoop() {
		timeval timeout;
		timeout.tv_usec = 50;
		fd_set fd;
		while (running_) {
			fd.fd_array[0] = serverSock_;
			fd.fd_count = 1;
			select(0, &fd, nullptr, nullptr, &timeout);
			if (fd.fd_count) {
				ptcl::Message msg = ptcl::receiveFromNet(serverSock_);

			}
		}
	}

	void ConnectionManager::connect() {
		Log::log() << "Connect called";
		if (running_)
			disconnect();
		running_ = true;
		serverSock_ = estabilishConnection();
		if (serverSock_)
			requestInfoForNewClient(serverSock_);
		listeningThread_ = std::thread(&ConnectionManager::listeningThreadLoop, this);
	}

	void ConnectionManager::disconnect() {
		Log::log() << "Disconnect called";
		if (running_) {
			running_ = false;
			listeningThread_.join();
			ptcl::Message byeMsg(ptcl::MessageType::EndConnection, 0);
			ptcl::sendThroughNet(byeMsg, serverSock_);
			closesocket(serverSock_);
		}
	}
}