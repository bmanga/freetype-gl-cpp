#include "VertexBuffer.h"


ftgl::VertexBuffer::VertexBuffer(const char* format):
	format(format)
{
	const char* start = format;
	const char* end = nullptr;
	size_t index = 0, stride = 0;
	GLchar *pointer = nullptr;
	do
	{
		cstring_view desc;
		uint32_t attribute_size = 0;
		end = strchr(start + 1, ',');
		if (end == nullptr)
		{
			desc = start;
		}
		else
		{
			desc.assign(start, end - start);
		}
		auto attribute = std::make_unique<VertexAttribute>(desc);
		start = end + 1;
		attribute->setPointer(pointer);
		switch (attribute->getType())
		{
		case GL_BOOL:           attribute_size = sizeof(GLboolean); break;
		case GL_BYTE:           attribute_size = sizeof(GLbyte); break;
		case GL_UNSIGNED_BYTE:  attribute_size = sizeof(GLubyte); break;
		case GL_SHORT:          attribute_size = sizeof(GLshort); break;
		case GL_UNSIGNED_SHORT: attribute_size = sizeof(GLushort); break;
		case GL_INT:            attribute_size = sizeof(GLint); break;
		case GL_UNSIGNED_INT:   attribute_size = sizeof(GLuint); break;
		case GL_FLOAT:          attribute_size = sizeof(GLfloat); break;
		default:                attribute_size = 0;
		}
		stride += attribute->getSize()*attribute_size;
		pointer += attribute->getSize()*attribute_size;
		attributes[index] = std::move(attribute);
		++index;
	} while (end && (index < MAX_VERTEX_ATTRIBUTES));
	
	for (size_t i = 0; i<index; ++i)
	{
		attributes[i]->setStride(stride);
	}
	
	vertex_stride = stride;
}

const char* ftgl::VertexBuffer::getFormat() const
{
	return format.c_str();
}

void ftgl::VertexBuffer::upload()
{
	if (state == State::FROZEN)
		return;

	if (!vertices_id)
	glGenBuffers(1, &vertices_id);

	if (!indices_id)
	glGenBuffers(1, &indices_id);

	size_t vsize = vertices.size() *
		sizeof(decltype(vertices)::value_type);

	size_t isize = indices.size() *
		sizeof(decltype(indices)::value_type);

	//Upload vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertices_id);

	if (vsize != GPU_vsize)
	{
		glBufferData(GL_ARRAY_BUFFER, vsize, vertices.data(), GL_DYNAMIC_DRAW);
		GPU_vsize = vsize;
	}
	else
	{
		glBufferSubData(GL_ARRAY_BUFFER, 0, vsize, vertices.data());
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Upload indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
	if (isize != GPU_isize)
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize,
		                                    indices.data(), GL_DYNAMIC_DRAW);
		GPU_isize = isize;
	}
	else
	{
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, isize, indices.data());
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ftgl::VertexBuffer::clear()
{
	state = State::FROZEN;
	indices.clear();
	vertices.clear();
	items.clear();
}

void ftgl::VertexBuffer::renderSetup(GLenum mode)
{
#ifdef FREETYPE_GL_USE_VAO
	// Unbind so no existing VAO-state is overwritten,
	// (e.g. the GL_ELEMENT_ARRAY_BUFFER-binding).
	glBindVertexArray(0);
#endif

	if (state != State::CLEAN)
	{
		upload();
		state = State::CLEAN;
	}

#ifdef FREETYPE_GL_USE_VAO
	if (VAO_id == 0)
	{
		// Generate and set up VAO

		glGenVertexArrays(1, &VAO_id);
		glBindVertexArray(VAO_id);

		glBindBuffer(GL_ARRAY_BUFFER, vertices_id);

		for (size_t i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
		{
			auto* attribute = attributes[i].get();
			if (!attribute)
			{
				continue;
			}
			else
			{
				attribute->enable();
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (!indices.empty())
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
		}
	}

	// Bind VAO for drawing
	glBindVertexArray(VAO_id);
#else

		glBindBuffer(GL_ARRAY_BUFFER, vertices_id);

		for (size_t i = 0; i<MAX_VERTEX_ATTRIBUTES; ++i)
		{
			auto* attribute = attributes[i].get();
			if (!attribute)
			{
				continue;
			}
			else
			{
				attribute->enable;
			}
		}

		if (!indices.empty())
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
		}
#endif

	mode = mode;
}


void ftgl::VertexBuffer::renderFinish()
{
#ifdef FREETYPE_GL_USE_VAO
	glBindVertexArray(0);
#else

		for (size_t i = 0; i<MAX_VERTEX_ATTRIBUTES; ++i)
		{
			auto* attribute = attributes[i].get();
			if (!attribute)
			{
				continue;
			}
			else
			{
				glDisableVertexAttribArray(attribute->getIndex());
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}

void ftgl::VertexBuffer::renderItem(size_t index)
{
	assert(index < items.size());

	auto item = items[index];
	if (!indices.empty())
	{
		glDrawElements(mode, item.icount, GL_UNSIGNED_INT,
		               (void*)(item.istart * sizeof(GLuint)));
	}
	else if (!vertices.empty())
	{
		glDrawArrays(mode,
		             item.vstart * sizeof(decltype(vertices)::value_type),
		             item.vcount);
	}
}

void ftgl::VertexBuffer::render(GLenum mode)
{
	size_t vcount = vertices.size();
	size_t icount = indices.size();
	renderSetup(mode);

	if (icount)
	{
		glDrawElements(mode, icount, GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		glDrawArrays(mode, 0, vcount);
	}
	renderFinish();
}

void ftgl::VertexBuffer::pushBackIndices(const GLuint* pindices, size_t icount)
{
	state |= State::DIRTY;
	//memcpy the indices at the end of the vector
	auto end = indices.size();
	indices.resize(end + icount);
	memcpy(&indices[end], pindices, icount * sizeof(GLuint));
}

void ftgl::VertexBuffer::pushBackVertices(const char* pvertices, size_t vcount)
{
	state |= State::DIRTY;
	vertices.insert(vertices.end(), 
		pvertices,
		pvertices + vcount * vertex_stride);
}

void ftgl::VertexBuffer::insertIndices(size_t index, const GLuint* pindices, size_t icount)
{
	assert(index <= indices.size());

	indices.insert(indices.end(), pindices, pindices + icount);
}

void ftgl::VertexBuffer::insertVertices(size_t index, const char* pvertices, size_t vcount)
{
	assert(index <= vertices.size());

	state |= State::DIRTY;
	//update the indices
	for (auto&& idx : indices)
	{
		if (idx > index)
			idx += index;
	}

	vertices.insert(vertices.begin() + index,
	                pvertices, pvertices + vcount * vertex_stride);
}

void ftgl::VertexBuffer::eraseIndices(size_t first, size_t last)
{
	assert(first < last);
	assert(last < indices.size());

	state |= State::DIRTY;
	indices.erase(indices.begin() + first, indices.begin() + last);
}

void ftgl::VertexBuffer::eraseVertices(size_t first, size_t last)
{
	assert(first < last);
	assert(last * vertex_stride < vertices.size());

	state |= State::DIRTY;

	//update the indices
	for (auto&& idx : indices)
	{
		if (idx > first)
			idx -= (last - first);
	}

	first *= vertex_stride;
	last *= vertex_stride;
	vertices.erase(vertices.begin() + first, vertices.begin() + last);
}

size_t ftgl::VertexBuffer::push_back(const char* pvertices, size_t vcount, const GLuint* pindices, size_t icount)
{
	return insert(items.size(), pvertices, vcount, pindices, icount);
}

size_t ftgl::VertexBuffer::insert(size_t index, const char* pvertices, size_t vcount, const GLuint* pindices, size_t icount)
{
	assert(pvertices);
	assert(pindices);
	assert(index <= items.size());

	state = State::FROZEN;

	//push back the vertices
	size_t vstart = vertices.size() / vertex_stride;
	pushBackVertices(pvertices, vcount);

	//push back the indices
	size_t istart = indices.size();
	pushBackIndices(pindices, icount);

	//update the new indices to match to the right vertex
	for (size_t i = 0; i < icount; ++i)
	{
		indices[istart + i] += vstart;
	}

	//insert item
	ivec4 item{vstart, vcount, istart, icount};
	items.insert(items.begin() + index, item);

	state = State::DIRTY;

	return index;
}

void ftgl::VertexBuffer::erase(size_t index)
{
	assert(index < items.size());

	auto delItem = items[index];

	//update items
	for (auto&& item : items)
	{
		if (item.vstart > delItem.vstart)
		{
			item.vstart -= delItem.vcount;
			item.istart -= delItem.icount;
		}
	}
	state = State::FROZEN;
	eraseIndices(delItem.istart, delItem.istart + delItem.icount);
	eraseVertices(delItem.vstart, delItem.vstart + delItem.vcount);
	items.erase(items.begin() + index);
	state = State::DIRTY;
}
