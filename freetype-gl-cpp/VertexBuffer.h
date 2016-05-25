#pragma once
#include <vector>
#include <memory>
#include <string>
#include "vec234.h"
#include "opengl.h"
#include "VertexAttribute.h"

static constexpr int MAX_VERTEX_ATTRIBUTES = 16;
#define FREETYPE_GL_USE_VAO
namespace ftgl {
class VertexBuffer
{
private:
	std::string format;

	//vertices are a cluster of various attributes, such as color,
	// texture coords, and positions. stored as char
	std::vector<char> vertices;
	size_t vertex_stride = 0;

	std::vector<GLuint> indices;
	std::vector<ivec4> items;

	//TODO: replace with std::vector
	std::unique_ptr<VertexAttribute> attributes[MAX_VERTEX_ATTRIBUTES];
#ifdef FREETYPE_GL_USE_VAO
	uint32_t VAO_id = 0;
#endif
	/** GL identity of the vertices buffer. */
	uint32_t vertices_id = 0;

	/** GL identity of the indices buffer. */
	uint32_t indices_id = 0;

	/** Current size of the vertices buffer in GPU */
	size_t GPU_vsize = 0;

	/** Current size of the indices buffer in GPU*/
	size_t GPU_isize = 0;

	/** GL primitives to render. */
	GLenum mode = GL_TRIANGLES;

	/** Whether the vertex buffer needs to be uploaded to GPU memory. */
	enum State : uint8_t
	{
		CLEAN = 0,
		DIRTY = 1,
		FROZEN = 2
	};

	uint8_t state = State::DIRTY;


public:
	VertexBuffer(const char* format);

	const char* getFormat() const;
	size_t size() const
	{
		return items.size();
	}

	void upload();
	void clear();
	void renderItem(size_t index);
	void render(GLenum mode);
	void pushBackIndices(const GLuint* pindices, size_t icount);
	void pushBackVertices(const char* pvertices, size_t vcount);
	void insertIndices(size_t index, const GLuint* pindices, size_t count);
	void insertVertices(size_t index, const char* pvertices, size_t vcount);
	void eraseIndices(size_t first, size_t last);
	void eraseVertices(size_t first, size_t last);
	size_t push_back(const char* pvertices, size_t vcount,
	                 const GLuint* pindices, size_t icount);
	size_t insert(size_t index, const char* pvertices, size_t vcount,
	              const GLuint* pindices, size_t icount);
	void erase(size_t index);

private:
	void renderSetup(GLenum mode);
	void renderFinish();
};
} //namespace ftgl
