#include "Application.h"
#include "GLFW\glfw3.h"
#include "Log.h"
#include <sstream>
#include <fstream>
#include <thread>
#include <string>
#include <cassert>
#include <algorithm>
#include <functional>
#include <map>

namespace snake {

	namespace {

		enum class ApplicationState {
			Idle,
			PlayerControlsEditing,
		};

		ApplicationState appState = ApplicationState::Idle;

		volatile int playerControlKey = 0;

		void editPlayerKeys(std::pair<std::string, Player::Controls>& pair) {
			auto &[name, controls] = pair;
			std::cout << "Press keys for " << name << "'s controls while activating GLFW window:\n";
			appState = ApplicationState::PlayerControlsEditing;
			static constexpr char directions[4][10] = { "up: ", "down: ", "left: ", "right: " };
			int pressedKeys[4];
			for (int i = 0; i < 4; i++) {
				std::cout << directions[i];
				while (!playerControlKey);
				pressedKeys[i] = playerControlKey;
				playerControlKey = 0;
				std::cout << "Key num " << pressedKeys[i] << " has been scaned.\n";
			}
			appState = ApplicationState::Idle;
			controls.up_ = pressedKeys[0];
			controls.down_ = pressedKeys[1];
			controls.left_ = pressedKeys[2];
			controls.right_ = pressedKeys[3];
			std::cout << "Settings for " << name << " saved.\n";
		}				
	}

	Application::Application()
		: connection_(*this) {
		MainGraphics::parseShaders("C:\\Users\\Vrah666\\Documents\\Visual Studio 2017\\Projects\\SnakeOpenGL\\resources");
		GLFWwindow * window = graphics_.getWindow();
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scanCode, int action, int mods) -> void {
			static_cast<Application*>(glfwGetWindowUserPointer(window))->keyCallback(key, scanCode, action, mods);
		});
	}

	void Application::quit() {
		running_ = false;
		if (game_)
			game_->endGame();
		glfwSetWindowShouldClose(graphics_.getWindow(), true);
	}

	void Application::startGame() {
		if (game_) {
			std::cout << "Game currently in progress. Terminate? [N/Y] ";
			char answer;
			do {
				std::cin >> answer;
			} while (answer != 'Y' && answer != 'N');
			if (answer == 'N')
				return;
			game_->endGame();
		}
		listPlayers();
		{
			std::vector<typename decltype(players_)::value_type*> missingControls;
			std::for_each(players_.begin(), players_.end(), [ins = std::back_inserter(missingControls)](decltype(*players_.begin()) param) mutable -> void {
				const auto& controls = param.second;
				using valType = decltype(controls.up_);
				const valType * ptr = std::launder(reinterpret_cast<const valType*>(&controls));
				for (int i = 0; i < 4; ++i, ++ptr)
					if (!*ptr) {
						ins = &param;
						return;
					}
			});
			if (missingControls.size()) {
				std::cout << "One or more players are missing some keys. Would you like to set them now? [Y/N] ";
				char c;
				do {
					std::cin >> c;
				} while (c != 'N' && c != 'Y');
				if (c == 'Y')
					for (auto*ptr : missingControls)
						editPlayerKeys(*ptr);
			}
		}
		std::cout << "Press some stuff to proceed or N to return.\n";
		char c;
		std::cin >> c;
		if (c == 'N')
			return;
		Game::Size size;
		std::cout << "New game size, pls:\nwidth: ";
		std::cin >> size.width_;
		std::cout << "height: ";
		std::cin >> size.height_;
		game_ = std::make_unique<Game>(*this, players_.size(), size, players_);
	}

	void Application::addPlayer() {
		std::pair<std::string, Player::Controls> pair;
		std::cout << "Player's name, pls: ";
		std::getline(std::cin, pair.first);
		std::cout << "Would you like to set " << pair.first << "'s controls right now? [Y/N] ";
		char c;
		do {
			std::cin >> c;
		} while (c != 'N' && c != 'Y');
		if (c == 'Y')
			editPlayerKeys(pair);
		players_.emplace_back(std::move(pair));
	}

	void Application::removePlayer() {
		auto* pair = selectPlayer();
		if (!pair)
			return;
		if (pair == &players_.back()) {
			players_.pop_back();
			std::cout << pair->first << " is the last player. His recorde has been erased.\n";
		}
		else {
			std::cout << pair->first << " is somewhere in the middle, his record will be overwritten.\n";
			*pair = std::move(players_.back());
		}
	}

	std::pair<std::string, Player::Controls>* Application::selectPlayer() {
		if (!players_.size()) {
			std::cout << "There are no players registered!\n";
			return nullptr;
		}
		listPlayers();
		std::pair<std::string, Player::Controls> * pair = nullptr;
		std::cout << "Type name or index of the player who shall be selected, or \"exit\" to leave without any changes.\n";
		while (!pair) {
			std::string name;
			std::getline(std::cin, name);
			if (!name.compare("exit"))
				return nullptr;
			unsigned index = std::stoi(name);	 //It is possible to search for player by index into the vector
			if ((index && index < players_.size()) || name[0] == '0') {
				pair = &players_[index];
				break;
			}
			if (auto iter = std::find_if(players_.begin(), players_.end(), [&](decltype(*players_.begin()) const & param) -> bool {
				return !param.first.compare(name);			//search by name
			}); iter == players_.end())
				std::cout << "Player " << name << " not found!\n";
			else
				pair = &*iter;
		}
		return pair;
	}

	void Application::editControls() {
		auto *pair = selectPlayer();
		if (pair)
			editPlayerKeys(*pair);
	}

	void Application::listPlayers() const {
		std::size_t count = players_.size();
		if (!count)
			std::cout << "There are currently no players.\n";
		else {
			if (count == 1)
				std::cout << "There is one player:\n";
			else
				std::cout << "There are " << count << " players connected:\n";
			for (const auto&[name, controls] : players_)
				std::cout << name << '\n';
		}
	}

	void Application::connect() {
		connection_.connect();
	}

	void Application::disconnect() {
		connection_.disconnect();
	}

	void Application::processCommand(std::string_view command) {
		static const std::map<std::string_view, std::function<void(Application*)>> commands = {
			{ "quit", &Application::quit},
			{"start", &Application::startGame},
			{"add player", &Application::addPlayer},
			{"list players", &Application::listPlayers},
			{"edit controls", &Application::editControls},
			{"connect", &Application::connect},
			{"disconnect", &Application::disconnect}
		};
		if (commands.count(command))
			commands.at(command)(this);
		else
			std::cout << "Unknown command " << command << '\n';
		/*
		if (!command.compare("quit"))
			return quit();
		if (!command.compare("start"))
			return startGame();
		if (!command.compare("add player"))
			return addPlayer();
		if (!command.compare("list players"))
			return listPlayers();
		if (!command.compare("edit controls"))
			return editPlayerControls();
		if (!command.compare("connect"))
			return connection_.connect();
		if (!command.compare("disconnect"))
			return connection_.disconnect(); */
	}

	int Application::run() {
		running_ = true;
		auto processLambda = [](Application* app) -> void {
			std::string input;
			while (app->running_) {
				std::cout << "Application awaits your command: ";
				std::getline(std::cin, input);
				if (app->running_ && input.size())
					app->processCommand(input);
			}
		};
		std::thread commandThread(processLambda, this);

		for (; !glfwWindowShouldClose(graphics_.getWindow());) {
			glfwPollEvents();
			glfwSwapBuffers(graphics_.getWindow());
		}
		quit();
		commandThread.join();
		return 0;
	}


	void Application::keyCallback(int key, int scanCode, int action, int mods) {
		if (appState == ApplicationState::PlayerControlsEditing) {
			playerControlKey = key;
			return;
		}
		if (game_)
			game_->keyCallback(key, scanCode, action, mods);
	}
}