#pragma once
#include <string>
#include "opengl.h"
#include "Utility.h"

namespace ftgl {
class VertexAttribute
{
private:
	//TODO: maybe change to small string or something
	std::string name;
	int size = 0;
	int index = -1;

public:
	std::string getName() const
	{
		return name;
	}

	int getSize() const
	{
		return size;
	}

	int getIndex() const
	{
		return index;
	}

	GLenum getType() const
	{
		return type;
	}

	size_t getStride() const
	{
		return stride;
	}

	void setStride(size_t s)
	{
		stride = s;
	}

	bool isNormalized() const
	{
		return normalized;
	}

	void* getPointer() const
	{
		return pointer;
	}

	void(* getEnable_fn() const)(void*)
	{
		return enable_fn;
	}

private:
	GLenum type = 0;
	size_t stride = 0;
	bool normalized = false;
	void* pointer = nullptr;

	void(*enable_fn) (void*) = nullptr;

public:
	VertexAttribute() = default;

	VertexAttribute(std::string name,
		int size,
		GLenum type,
		bool normalizd,
		size_t stride,
		void* pointer);

	VertexAttribute(cstring_view format);

	bool isValid() const;
	void enable();
	void parse(cstring_view format);
	void setPointer(void* ptr);
};

}//namespace ftgl