#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include "Game.h"
#include "MainGraphics.h"
#include "ConnectionManager.h"
#include <string>
#include <filesystem>
#include <string_view>
#include <memory>
#include <vector>
#include <WS2tcpip.h>

namespace snake {

	class Application {
	public:
	private:
		MainGraphics graphics_;
		ConnectionManager connection_;
		std::unique_ptr<Game> game_;
		std::vector<std::pair<std::string, Player::Controls>> players_;
		volatile bool running_ = false;

		void processCommand(std::string_view command);

		std::pair<std::string, Player::Controls>* selectPlayer();

		void quit();
		void connect();
		void disconnect();
		void startGame();
		void addPlayer();
		void removePlayer();
		void listPlayers() const;
		void editControls();
	public:
		Application();

		int run();
		void keyCallback(int key, int scanCode, int action, int mods);

		MainGraphics& graphics() { return graphics_; }
	};
}


#endif //^^APPLICATION_H							   