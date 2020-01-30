#pragma once
#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <WS2tcpip.h>
#include <thread>		   

namespace snake {					

	class Application;

	class ConnectionManager {
	private:
		Application & myApp_;
		SOCKET serverSock_ = INVALID_SOCKET;
		volatile bool running_ = false;
		std::thread listeningThread_;

		void listeningThreadLoop();

	public:
		ConnectionManager(Application& app);
		~ConnectionManager();

		void connect();
		void disconnect();	 

	};	

}
							 
#endif //^^^CONNECTIONMANAGER_H