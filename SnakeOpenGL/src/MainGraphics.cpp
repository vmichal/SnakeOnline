#include "MainGraphics.h"
#include "Log.h"
#include <string>
#include <cassert>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

	struct ShaderSources {
		std::string vertexShader_, fragmentShader_;
	};

	ShaderSources readShaders(std::filesystem::path& path) {
		std::filesystem::path vertexPath = path.string() + "/vertex.shader",
			fragmentPath = path.string() + "/fragment.shader";
		if (!std::filesystem::exists(vertexPath) || !std::filesystem::exists(fragmentPath))
			throw std::runtime_error("Shaders do not exist!"); //TODO vymyslet error checking
		ShaderSources sources;
		sources.vertexShader_.resize(static_cast<unsigned>(std::filesystem::file_size(vertexPath)));
		sources.fragmentShader_.resize(static_cast<unsigned>(std::filesystem::file_size(fragmentPath)));
		std::ifstream(vertexPath).read(sources.vertexShader_.data(), sources.vertexShader_.capacity());
		std::ifstream(fragmentPath).read(sources.fragmentShader_.data(), sources.fragmentShader_.capacity());				
		return sources;
	}

	GLFWwindow *createWindow() {
		Log::log() << "Creating window ";
		GLFWwindow * window = glfwCreateWindow(1000, 600, "Window", nullptr, nullptr);

		assert(window);

		glfwMakeContextCurrent(window);

		assert(glewInit() == GLEW_OK);
		Log::log("Window creation successfull");
		return window;
	}

	unsigned compileShader(unsigned type, std::string_view source) {
		unsigned shader = glCreateShader(type);
		const char * src = source.data();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		int result;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		if (!result) {
			int length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			std::vector<char> buffer(length);
			glGetShaderInfoLog(shader, length, &length, buffer.data());

			Log::error() << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader.\n"
				<< buffer.data();
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}

	unsigned createShaderProgram(const ShaderSources& shaders) {
		unsigned vertexShader = compileShader(GL_VERTEX_SHADER, shaders.vertexShader_);
		assert(vertexShader);
		unsigned fragmentShader = compileShader(GL_FRAGMENT_SHADER, shaders.fragmentShader_);
		assert(fragmentShader);
		unsigned program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		glLinkProgram(program);
		glValidateProgram(program);

		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);

		return program;
	}

	unsigned createVertexBuffer() {
		unsigned buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(snake::Vertex), (const void*)0);
		return buffer;
	}

	unsigned createIndexBuffer() {
		unsigned buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		return buffer;
	}
}

#include <vector>

namespace snake {

	void MainGraphics::parseShaders(std::filesystem::path& path) {
		ShaderSources sources = readShaders(path);
		shaderProgram_ = createShaderProgram(sources);
		sizeof(std::vector<int>::iterator);
	}

	MainGraphics::MainGraphics()
		: window_(createWindow()), vertexBuffer_(createVertexBuffer()), indexBuffer_(createIndexBuffer()) {
		glUseProgram(shaderProgram_);
		Log::log() << "OpenGL version " << glGetString(GL_VERSION);
	}

	MainGraphics::~MainGraphics() {
		glfwDestroyWindow(window_);
	}


	void MainGraphics::resize(unsigned colCount, unsigned rowCount) {
		std::vector<Vertex> vertices(colCount * rowCount);
		std::vector<Vertex>::iterator iter = vertices.begin();
		const float xSpan = 2.0f / colCount,
			ySpan = 2.0f / rowCount;
		for (int y = 0; y < rowCount; ++y)
			for (int x = 0; x < colCount; ++iter, ++x)
				*iter = { xSpan*x - 1, 1 - ySpan * y };
		glBufferData(GL_ARRAY_BUFFER, vertices.size(), vertices.data(), GL_STATIC_DRAW);
	}

	void MainGraphics::draw(unsigned count) const {
		if (glfwWindowShouldClose(window_)) {
			Log::warning() << "Window should close";
			return;
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		glfwSwapBuffers(window_);
		glfwPollEvents();
	}
}