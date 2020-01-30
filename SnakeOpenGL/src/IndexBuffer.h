#pragma once
#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include <cstddef>

class IndexBuffer {

private:
	unsigned handle_;
	std::size_t size_;

public:
	IndexBuffer();
	IndexBuffer(const unsigned *data, std::size_t size);
	~IndexBuffer();

	void bind();
	void unbind();

	void data(const unsigned * data, std::size_t size);

};

#endif //VERTEXBUFFER_H