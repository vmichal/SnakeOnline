#include <WS2tcpip.h>
#include <iostream>
#include "Server.h"
#include "Log.h"

#pragma comment (lib, "ws2_32.lib")


int main(int argc, char** argv) {
	Log::initialize("OpenGLSnakeServer", "snk");
	WSADATA WSAdata;
	if (int errCode = WSAStartup(MAKEWORD(2, 2), &WSAdata)) {
		Log::error() << "Can't init winsock! Exit code " << errCode;
		return errCode;
	}
	Log::log() << WSAdata.szDescription << '\n' << WSAdata.szSystemStatus;
	Log::prompt("Èíslo portu plox");
	unsigned short port;
	Log::input() >> port;
	Log::log() << "Max number of server sockets " << WSAdata.iMaxSockets;
	int returnCode;
	{
		Server server(port);
		returnCode = server.run();
	}
	WSACleanup();
	return returnCode;
}