#pragma once
#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <cstddef>

class VertexBuffer {

private:
	unsigned handle_;
	std::size_t size_;

public:
	VertexBuffer();
	VertexBuffer(const void *data, std::size_t size);
	~VertexBuffer();

	void bind();
	void unbind();

	void data(const void * data, std::size_t size);

};

#endif //VERTEXBUFFER_H