#pragma once
#ifndef MAINGRAPHICS_H
#define MAINGRAPHICS_H

#include "GL\glew.h"
#include "GLFW\glfw3.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include <filesystem>
#include <map>

namespace snake {	 

	struct Vertex {
		float x_, y_;
	};					

	struct Color {
		float red_, green_, blue_, alpha_;
	};

	class Player;

	class MainGraphics {
	private:
		static unsigned shaderProgram_;
		GLFWwindow * window_;
		VertexBuffer vertices_;
		IndexBuffer food_;		
		std::map<Player*, IndexBuffer> players_;

		unsigned vertexBuffer_;  

	public:
		static void parseShaders(const std::filesystem::path& shaders);

		MainGraphics();
		~MainGraphics();

		void supplyFoodCoordinates(Vertex vertices[4]);

		void draw(unsigned count) const;

		void resize(unsigned x, unsigned y);

		GLFWwindow * getWindow() {
			return window_;
		}
		const GLFWwindow * getWindow() const {
			return window_;
		}

	};
}

#endif //^^MAINGRAPHICS_H
