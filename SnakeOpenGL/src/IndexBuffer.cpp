#include "IndexBuffer.h"
#include "GL/glew.h"

IndexBuffer::IndexBuffer() {
	glGenBuffers(1, &handle_);
}

IndexBuffer::~IndexBuffer() {
	glDeleteBuffers(1, &handle_);
}

IndexBuffer::IndexBuffer(const unsigned*data, std::size_t size)
	: IndexBuffer() {	
	size_ = size;
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void IndexBuffer::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, handle_);
}

void IndexBuffer::unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void IndexBuffer::data(const unsigned *data, std::size_t size) {
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}
														   