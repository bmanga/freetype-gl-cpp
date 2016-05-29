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
 *
 * This source is based on the article by Jukka Jylänki :
 * "A Thousand Ways to Pack the Bin - A Practical Approach to
 * Two-Dimensional Rectangle Bin Packing", February 27, 2010.
 *
 * More precisely, this is an implementation of the Skyline Bottom-Left
 * algorithm based on C++ sources provided by Jukka Jylänki at:
 * http://clb.demon.fi/files/RectangleBinPack/
 *
 *  ============================================================================
 */
#pragma once

#include <cstdlib>
#include <vector>
#include "vec234.h"
#include <memory>

//#include "vector.h"

namespace ftgl {


/**
 * @file   texture-atlas.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup texture-atlas Texture atlas
 *
 * A texture atlas is used to pack several small regions into a single texture.
 *
 * The actual implementation is based on the article by Jukka Jylänki : "A
 * Thousand Ways to Pack the Bin - A Practical Approach to Two-Dimensional
 * Rectangle Bin Packing", February 27, 2010.
 * More precisely, this is an implementation of the Skyline Bottom-Left
 * algorithm based on C++ sources provided by Jukka Jylänki at:
 * http://clb.demon.fi/files/RectangleBinPack/
 *
 *
 * Example Usage:
 * @code
 * #include "texture-atlas.h"
 *
 * ...
 *
 * / Creates a new atlas of 512x512 with a depth of 1
 * texture_atlas_t * atlas = texture_atlas_new( 512, 512, 1 );
 *
 * // Allocates a region of 20x20
 * ivec4 region = texture_atlas_get_region( atlas, 20, 20 );
 *
 * // Fill region with some data
 * texture_atlas_set_region( atlas, region.x, region.y, region.width, region.m_height, data, stride )
 *
 * ...
 *
 * @endcode
 *
 * @{
 */

class TextureAtlas
{
	using Node = ftgl::ivec3;
	using Nodes = std::vector<Node>;
private:
	/**
	* Allocated nodes
	*/
	Nodes m_nodes;

	/**
	*  Width (in pixels) of the underlying texture
	*/
	size_t m_width;

	/**
	* Height (in pixels) of the underlying texture
	*/
	size_t m_height;

	/**
	* Depth (in bytes) of the underlying texture
	*/
	size_t m_depth;

	/**
	* Allocated surface size
	*/
	size_t m_used;

	/**
	* Texture identity (OpenGL)
	*/
	unsigned int m_id;

	/**
	* Atlas data. unique_ptr with custom deleter (free)
	*/
	std::unique_ptr<unsigned char, decltype(std::free)*> m_data;

	/**
	* the texture is dirty and needs reuploading
	*/
	bool m_dirty = true;

public:
	TextureAtlas(size_t width, size_t height, size_t depth);
	~TextureAtlas();

	/**
	* getters
	*/

	size_t width() const { return m_width; }
	size_t height() const { return m_height; }
	size_t depth() const { return m_depth; }
	unsigned id() const { return m_id; }
	const void* data() const { return m_data.get(); }
	const Nodes& nodes() const { return m_nodes; }
	Nodes& nodes() { return m_nodes; }

	/**
	*  Upload atlas to video memory.
	*/
	void upload();

	/**
	*  Allocate a new region in the atlas.
	*
	*  @param width  width of the region to allocate
	*  @param m_height m_height of the region to allocate
	*  @return       Coordinates of the allocated region
	*
	*/
	ftgl::ivec4 getRegion(size_t width, size_t height);

	/**
	*  Upload data to the specified atlas region.
	*
	*  @param x      x coordinate the region
	*  @param y      y coordinate the region
	*  @param width  width of the region
	*  @param m_height m_height of the region
	*  @param data   data to be uploaded into the specified region
	*  @param stride stride of the data
	*
	*/
	void setRegion(size_t x, size_t y, size_t width, size_t height,
		const unsigned char* data, size_t stride);

	/**
	*  Remove all allocated regions from the atlas.
	*/
	void clear();

private:
	int fit(size_t index, size_t width, size_t height);
	void merge();
};

}// namespace ftgl