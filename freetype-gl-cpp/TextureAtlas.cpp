/* ============================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         https://github.com/rougier/freetype-gl
 * ----------------------------------------------------------------------------
 * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier.
 * ============================================================================
 */


#include <cassert>
#include <limits>
#include "TextureAtlas.h"
#include "opengl.h"

ftgl::TextureAtlas::TextureAtlas(size_t width, size_t height, size_t depth) :
	m_width(width), m_height(height), m_depth(depth), m_used(0), m_id(0),
	m_data(static_cast<unsigned char*>(calloc(
			       width*height*depth, sizeof(unsigned char))
	       ), free)
{
	// We want a one pixel border around the whole atlas to avoid any artefact when
	// sampling texture
	m_nodes.push_back(Node{ 1, 1, int(width) - 2 });

	assert((depth == 1) || (depth == 3) || (depth == 4));

	if (m_data == nullptr)
	{
		fprintf(stderr,
			"line %d: No more memory for allocating data\n", __LINE__);
		exit(EXIT_FAILURE);
	}
}

ftgl::TextureAtlas::~TextureAtlas()
{
	if (m_id)
	{
		glDeleteTextures(1, &m_id);
	}
}

void ftgl::TextureAtlas::setRegion(
	size_t x,
	size_t y,
	size_t width,
	size_t height,
	const unsigned char * data,
	size_t stride)
{
	assert(x > 0);
	assert(y > 0);
	assert(x < (m_width - 1));
	assert((x + width) <= (m_width - 1));
	assert(y < (m_height - 1));
	assert((y + height) <= (m_height - 1));

	m_dirty = true;

	size_t charsize = sizeof(char);

	for (size_t i = 0; i < height; ++i)
	{
		memcpy(m_data.get() + ((y + i)*m_width + x) * charsize * m_depth,
			   data + (i*stride) * charsize, 
			   width * charsize * m_depth);
	}
}


int ftgl::TextureAtlas::fit(
	      size_t index,
	const size_t width,
	const size_t height)
{

	int width_left = width;
	auto node = m_nodes[index];
	int x = node.x;
	int y = node.y;


	if ((x + width) > (m_width - 1))
	{
		return -1;
	}

	while (width_left > 0)
	{
		assert(index < m_nodes.size());

		node = m_nodes[index];
		if (node.y > y)
		{
			y = node.y;
		}
		if ((y + height) > (m_height - 1))
		{
			return -1;
		}
		width_left -= node.z;
		++index;
	}
	return y;
}

void ftgl::TextureAtlas::merge()
{
	for (size_t i = 0; i < m_nodes.size() - 1; ++i)
	{
		auto& node = m_nodes[i];
		auto& next = m_nodes[i + 1];

		if (node.y == next.y)
		{
			node.z += next.z;
			m_nodes.erase(m_nodes.begin() + i + 1);
			--i;
		}
	}
}


ftgl::ivec4 ftgl::TextureAtlas::getRegion(size_t width, size_t height)
{
	ftgl::ivec4 region = { {0, 0, int(width), int(height)} };

	size_t best_height = std::numeric_limits<size_t>::max();
	int best_index = -1;
	size_t best_width = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < m_nodes.size(); ++i)
	{
		int y = fit(i, width, height);
		if (y >= 0)
		{
			auto node = m_nodes[i];
			if (((y + height) < best_height)
				|| (((y + height) == best_height)
					&& (node.z > 0
						&& size_t(node.z) < best_width)))
			{
				best_height = y + height;
				best_index = i;
				best_width = node.z;
				region.x = node.x;
				region.y = y;
			}
		}
	}

	if (best_index == -1)
	{
		region.x = -1;
		region.y = -1;
		region.width = 0;
		region.height = 0;
		return region;
	}

	//new_node.x = region.x;
	//new_node.y = region.y + height;
	//new_node.z = width;
	Node new_node{ region.x, region.y + int(height), int(width) };

	m_nodes.insert(m_nodes.begin() + best_index, new_node);

	for (size_t i = best_index + 1; i < m_nodes.size(); ++i)
	{
		auto& node = m_nodes[i];
		auto& prev = m_nodes[i - 1];

		if (node.x >= (prev.x + prev.z)) break;

		int shrink = prev.x + prev.z - node.x;
		node.x += shrink;
		node.z -= shrink;

		if (node.z > 0) break;

		m_nodes.erase(m_nodes.begin() + i);
		--i;
	}
	merge();
	m_used += width * height;
	return region;
}


void ftgl::TextureAtlas::clear()
{
	m_used = 0;
	m_dirty = true;
	
	m_nodes.clear();

	// We want a one pixel border around the whole atlas to avoid any artefact when
	// sampling texture
	m_nodes.push_back(Node{ 1, 1, int(m_width) - 2 });

	// Clear out the data
	memset(m_data.get(), 0, m_width*m_height*m_depth);
}

void ftgl::TextureAtlas::upload()
{
	if (!m_dirty) return;

	if (!m_id)
	{
		glGenTextures(1, &m_id);
	}

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (m_depth == 4)
	{
#ifdef GL_UNSIGNED_INT_8_8_8_8_REV
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height,
			0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data());
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height,
			0, GL_RGBA, GL_UNSIGNED_BYTE, data());
#endif
	}
	else if (m_depth == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height,
			0, GL_RGB, GL_UNSIGNED_BYTE, data());
	}
	else
	{
#if defined(GL_ES_VERSION_2_0) || defined(GL_ES_VERSION_3_0)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_width, m_height,
			0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data());
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height,
			0, GL_RED, GL_UNSIGNED_BYTE, data());
#endif
	}

	m_dirty = false;
}

