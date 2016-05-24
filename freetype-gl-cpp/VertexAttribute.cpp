#include "VertexAttribute.h"
#include <cstring>
#include <cassert>

ftgl::VertexAttribute::VertexAttribute(std::string name, int size, 
	GLenum type, bool normalized, size_t stride, void* pointer) :
	name(std::move(name)),
	size(size),
	type(type),
	stride(stride),
	normalized(normalized),
	pointer(pointer)
{
}

ftgl::VertexAttribute::VertexAttribute(cstring_view format)
{
	parse(format);
}

bool ftgl::VertexAttribute::isValid() const
{
	return name.empty();
}

void ftgl::VertexAttribute::enable()
{
	if (index == -1)
	{
		GLint program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &program);
		if (program == 0)
		{
			return;
		}

		index = glGetAttribLocation(program, name.c_str());

		if (index == -1)
		{
			return;
		}
	}
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type,
		normalized, stride, pointer);
}

void ftgl::VertexAttribute::parse(cstring_view format)
{
	/*
	 *  format: [name]:[size][type][normalized]opt
	 */
	char ctype;
	const char *p = format.findPtr(':');

	if(p)
	{
		name.assign(format, p - format);

		if (*(++p) == '\0')
		{
			fprintf(stderr, 
				"No size specified for '%s' attribute\n", name.c_str());
			name.clear();
			return;
		}
		
		size = *p - '0';

		if (*(++p) == '\0')
		{
			fprintf(stderr, 
				"No format specified for '%s' attribute\n", name.c_str());
			name.clear();
			return;
		}
		
		ctype = *p;

		if (*(++p) != '\0')
		{
			if (*p == 'n')
			{
				normalized = 1;
			}
		}
	}
	else
	{
		fprintf(stderr, 
			"Vertex attribute format not understood ('%s')\n", 
			(const char*)format);
		return;
	}

	switch (ctype)
	{
	case 'b': type = GL_BYTE;           break;
	case 'B': type = GL_UNSIGNED_BYTE;  break;
	case 's': type = GL_SHORT;          break;
	case 'S': type = GL_UNSIGNED_SHORT; break;
	case 'i': type = GL_INT;            break;
	case 'I': type = GL_UNSIGNED_INT;   break;
	case 'f': type = GL_FLOAT;          break;
	default:  type = 0;                 break;
	}
}

void ftgl::VertexAttribute::setPointer(void* ptr)
{
	pointer = ptr;
}
