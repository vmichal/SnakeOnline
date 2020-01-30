#include "VertexBuffer.h"
#include "GL/glew.h"

VertexBuffer::VertexBuffer() {
	glGenBuffers(1, &handle_);
}

VertexBuffer::~VertexBuffer() {
	glDeleteBuffers(1, &handle_);
} 

VertexBuffer::VertexBuffer(const void*data, std::size_t size)
	: VertexBuffer() {				
}

void VertexBuffer::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, handle_);
}

void VertexBuffer::unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::data(const void *data, std::size_t size) {
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}
														   