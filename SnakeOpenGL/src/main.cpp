#include "Application.h"
#include "Log.h"  
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

//Client main

#include <vector>
#include <type_traits>
#include <string>

int init() {	
	Log::initialize("OpenGLSnakeClient", "snk");
	WSAData wsaData;
	if (int errCode = WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		Log::error() << "Can't init Winsock! Exit code " << errCode;
		return errCode;
	}

	if (glfwInit() != GLFW_TRUE) {
		Log::error("Unable to initialize GLFW!");
		WSACleanup();
		return GLFW_FALSE;
	}
	return 0;
}

int main() {
	if (int errCode = init())
		return errCode;
	int returnCode;
	try {
		snake::Application app;
		returnCode = app.run();
	}
	catch (std::runtime_error& err) {
		Log::error() << err.what();
		returnCode = ~0;
	}
	glfwTerminate();
	WSACleanup();
	return returnCode;
}