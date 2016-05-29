/* ===========================================================================
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
#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_LCD_FILTER_H

#include <cstdint>
#include <cassert>
#include <algorithm>

#include "utf8Utils.h"
#include "TextureFont.h"

static constexpr int   HRES  = 64;
static constexpr float HRESf = 64.f;
static constexpr int   DPI   = 72;

#define FTGL_STDERR_DISPLAY

#ifdef FTGL_STDERR_DISPLAY
#include "FT_Errors.h"
#endif


float ftgl::Glyph::getKerning(const char * codepoint) const
{
	return getKerning(utf8_to_utf32(codepoint));
}

float ftgl::Glyph::getKerning(uint32_t ucodepoint) const
{
	auto k_it = std::find_if(kernings.begin(), kernings.end(),
		[ucodepoint](const Kerning& kerning)
	{
		return kerning.codepoint == ucodepoint;
	});

	if (k_it != kernings.end())
		return k_it->kerning;

	return 0;
}

/*******************Font*****************/


ftgl::Font::Font(TextureAtlas* atlas, float pt_size, File file) : 
	m_atlas(atlas),
	m_location(TEXTURE_FONT_FILE),
	m_filename(file.filename),
	m_size(pt_size)
{
	m_success = init();
}


ftgl::Font::Font(TextureAtlas* atlas, float pt_size, Memory memory) : 
	m_atlas(atlas),
	m_location(TEXTURE_FONT_MEMORY), 
	m_memory{ memory.memory_base, memory.memory_size },
	m_size(pt_size)
{
	assert(m_memory.base);
	assert(m_memory.size);

	m_success = init();
}

bool ftgl::Font::init()
{
	FT_Library library;
	FT_Face face;

	assert(m_size > 0);
	assert((m_location == TEXTURE_FONT_FILE && 
		    m_filename.generic_string().size())
		|| (m_location == TEXTURE_FONT_MEMORY
			&& m_memory.base && m_memory.size));


	if (!loadFace(m_size * 100.f, &library, &face))
		return false;

	m_underline_position = face->underline_position / (float)(HRESf*HRESf) * m_size;
	m_underline_position = round(m_underline_position);

	if (m_underline_position > -2)
	{
		m_underline_position = -2.0;
	}

	m_underline_thickness = face->underline_thickness / (float)(HRESf*HRESf) * m_size;
	m_underline_thickness = round(m_underline_thickness);
	if (m_underline_thickness < 1)
	{
		m_underline_thickness = 1.0;
	}

	FT_Size_Metrics metrics = face->size->metrics;
	m_ascender = (metrics.ascender >> 6) / 100.0f;
	m_descender = (metrics.descender >> 6) / 100.0f;
	m_height = (metrics.height >> 6) / 100.0f;
	m_linegap = m_height - m_ascender + m_descender;
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	/* NULL is a special glyph */
	getGlyph(nullptr);

	return true;
}

const ftgl::Glyph*
ftgl::Font::getGlyph(const char* codepoint)
{
	uint32_t ucodepoint = ftgl::utf8_to_utf32(codepoint);
	Glyph* glyph = nullptr;

	assert(m_filename.generic_string().size());

	/* Check if codepoint has been already loaded */
	if ((glyph = findGlyph(ucodepoint)))
		return glyph;

	/* codepoint nullptr is special : it is used for line drawing (overline,
	* underline, strikethrough) and background.
	*/
	if (!codepoint)
	{
		size_t width = m_atlas->width();
		size_t height = m_atlas->height();
		ivec4 region = m_atlas->getRegion(5, 5);
		Glyph new_glyph;
		static constexpr unsigned char data[4 * 4 * 3]{
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

		if (region.x < 0)
		{
		#ifdef FTGL_STDERR_DISPLAY
			fprintf(stderr, "Texture atlas is full (line %d)\n", __LINE__);
		#endif
			return nullptr;
		}
		m_atlas->setRegion(region.x, region.y, 4, 4, data, 0);
		new_glyph.codepoint = -1;
		new_glyph.s0 = (region.x + 2) / (float)width;
		new_glyph.t0 = (region.y + 2) / (float)height;
		new_glyph.s1 = (region.x + 3) / (float)width;
		new_glyph.t1 = (region.y + 3) / (float)height;
		m_glyphs.push_back(new_glyph);
		return &m_glyphs.back();
	}

	/* Glyph has not been already loaded */
	if (loadGlyphs(codepoint) == 0)
	{
		return findGlyph(ucodepoint);
	}
	return nullptr;
}

const ftgl::Glyph* ftgl::Font::getLoadedGlyph(uint32_t ucodepoint)
{
	return findGlyph(ucodepoint);
}

size_t ftgl::Font::loadGlyphs(const char* codepoints)
{
	assert(codepoints);


	FT_Library library;
	FT_Face face;
	FT_Glyph ft_glyph = nullptr;
	FT_GlyphSlot slot;
	FT_Bitmap ft_bitmap;

	FT_UInt glyph_index;
	FT_Int32 flags = 0;
	int ft_glyph_top = 0;
	int ft_glyph_left = 0;

	ivec4 region;
	size_t missed = 0;


	auto width = m_atlas->width();
	auto height = m_atlas->height();
	auto depth = m_atlas->depth();

	if (!loadFace(m_size, &library, &face))
		return utf8_strlen(codepoints);

	/* Load each glyph */
	for (size_t i = 0; i < utf8_strlen(codepoints); i += utf8_surrogate_len(codepoints + i)) {
		uint32_t ucodepoint = utf8_to_utf32(codepoints + i);
		/* Check if codepoint has been already loaded */
		if (findGlyph(ucodepoint))
			continue;

		flags = 0;
		ft_glyph_top = 0;
		ft_glyph_left = 0;
		glyph_index = FT_Get_Char_Index(face, (FT_ULong)utf8_to_utf32(codepoints + i));
		// WARNING: We use texture-atlas depth to guess if user wants
		//          LCD subpixel rendering


		if (unsigned char(m_outline_type) > 0)
		{
			flags |= FT_LOAD_NO_BITMAP;
		}
		else
		{
			flags |= FT_LOAD_RENDER;
		}

		if (!m_hinting)
		{
			flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
		}
		else
		{
			flags |= FT_LOAD_FORCE_AUTOHINT;
		}

		if (depth == 3)
		{
			FT_Library_SetLcdFilter(library, FT_LCD_FILTER_LIGHT);
			flags |= FT_LOAD_TARGET_LCD;

			if (m_filtering)
			{
				FT_Library_SetLcdFilterWeights(library, m_lcd_weights);
			}
		}

		FT_Error error = FT_Load_Glyph(face, glyph_index, flags);
		if (error)
		{
		#ifdef FTGL_STDERR_DISPLAY
			fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
				__LINE__, FT_Errors[error].code, FT_Errors[error].message);
		#endif
			FT_Done_Face(face);
			FT_Done_FreeType(library);
			return utf8_strlen(codepoints) - utf8_strlen(codepoints + i);
		}


		if (unsigned char(m_outline_type) == 0)
		{
			slot = face->glyph;
			ft_bitmap = slot->bitmap;
			ft_glyph_top = slot->bitmap_top;
			ft_glyph_left = slot->bitmap_left;
		}
		else
		{
			FT_Stroker stroker;
			FT_BitmapGlyph ft_bitmap_glyph;
			error = FT_Stroker_New(library, &stroker);
			if (error)
			{
			#ifdef FTGL_STDERR_DISPLAY
				fprintf(stderr, "FT_Error (0x%02x) : %s\n",
					FT_Errors[error].code, FT_Errors[error].message);
			#endif

				FT_Done_Face(face);
				FT_Stroker_Done(stroker);
				FT_Done_FreeType(library);
				return 0;
			}
			FT_Stroker_Set(stroker,
				(int)(m_outline_thickness * HRES),
				FT_STROKER_LINECAP_ROUND,
				FT_STROKER_LINEJOIN_ROUND,
				0);
			error = FT_Get_Glyph(face->glyph, &ft_glyph);
			if (error)
			{
			#ifdef FTGL_STDERR_DISPLAY
				fprintf(stderr, "FT_Error (0x%02x) : %s\n",
					FT_Errors[error].code, FT_Errors[error].message);
			#endif

				FT_Done_Face(face);
				FT_Stroker_Done(stroker);
				FT_Done_FreeType(library);
				return 0;
			}

			if (unsigned char(m_outline_type) == 1)
			{
				error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
			}
			else if (unsigned char(m_outline_type) == 2)
			{
				error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 0, 1);
			}
			else if (unsigned char(m_outline_type) == 3)
			{
				error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 1, 1);
			}
			if (error)
			{
			#ifdef FTGL_STDERR_DISPLAY
				fprintf(stderr, "FT_Error (0x%02x) : %s\n",
					FT_Errors[error].code, FT_Errors[error].message);
			#endif

				FT_Done_Face(face);
				FT_Stroker_Done(stroker);
				FT_Done_FreeType(library);
				return 0;
			}

			if (depth == 1)
			{
				error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
				if (error)
				{
				#ifdef FTGL_STDERR_DISPLAY
					fprintf(stderr, "FT_Error (0x%02x) : %s\n",
						FT_Errors[error].code, FT_Errors[error].message);
				#endif

					FT_Done_Face(face);
					FT_Stroker_Done(stroker);
					FT_Done_FreeType(library);
					return 0;
				}
			}
			else
			{
				error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
				if (error)
				{
				#ifdef FTGL_STDERR_DISPLAY
					fprintf(stderr, "FT_Error (0x%02x) : %s\n",
						FT_Errors[error].code, FT_Errors[error].message);
				#endif

					FT_Done_Face(face);
					FT_Stroker_Done(stroker);
					FT_Done_FreeType(library);
					return 0;
				}
			}

			ft_bitmap_glyph = (FT_BitmapGlyph)ft_glyph;
			ft_bitmap = ft_bitmap_glyph->bitmap;
			ft_glyph_top = ft_bitmap_glyph->top;
			ft_glyph_left = ft_bitmap_glyph->left;
			FT_Stroker_Done(stroker);
		}

		// We want each glyph to be separated by at least one black pixel
		size_t w = ft_bitmap.width / depth;
		size_t h = ft_bitmap.rows;

		region = m_atlas->getRegion(w + 1, h + 1);
		if (region.x < 0)
		{	
		#ifdef FTGL_STDERR_DISPLAY
			fprintf(stderr, "Texture atlas is full (line %d)\n", __LINE__);
		#endif

			missed++;
			continue;
		}
		size_t x = region.x;
		size_t y = region.y;
		m_atlas->setRegion(x, y, w, h, ft_bitmap.buffer, ft_bitmap.pitch);

		Glyph glyph;
		glyph.codepoint = ucodepoint;
		glyph.width = w;
		glyph.height = h;
		glyph.outline_type = m_outline_type;
		glyph.outline_thickness = m_outline_thickness;
		glyph.offset_x = ft_glyph_left;
		glyph.offset_y = ft_glyph_top;
		glyph.s0 = x / float(width);
		glyph.t0 = y / float(height);
		glyph.s1 = (x + glyph.width) / float(width);
		glyph.t1 = (y + glyph.height) / float(height);

		// Discard hinting to get advance
		FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
		slot = face->glyph;
		glyph.advance_x = slot->advance.x / HRESf;
		glyph.advance_y = slot->advance.y / HRESf;

		m_glyphs.push_back(std::move(glyph));

		if (unsigned char(m_outline_type) > 0)
		{
			FT_Done_Glyph(ft_glyph);
		}
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);
	generateKerning();

	return missed;
}

bool ftgl::Font::loadFace(float size, FT_Library *library, FT_Face *face) const
{
	FT_Matrix matrix = {
		(int)((1.0 / HRES) * 0x10000L),
		(int)((0.0) * 0x10000L),
		(int)((0.0) * 0x10000L),
		(int)((1.0) * 0x10000L) };

	assert(library);
	assert(size);

	/* Initialize library */
	FT_Error error = FT_Init_FreeType(library);
	if (error) {
	#ifdef FTGL_STDERR_DISPLAY
		fprintf(stderr, "FT_Error (0x%02x) : %s\n",
		        FT_Errors[error].code, FT_Errors[error].message);
	#endif
		return false;
	}

	/* Load face */
	switch (m_location) 
	{
	case TEXTURE_FONT_FILE:
		error = FT_New_Face(*library, m_filename.generic_string().c_str(),
			0, face);
		break;

	case TEXTURE_FONT_MEMORY:
		error = FT_New_Memory_Face(*library,
			static_cast<const FT_Byte*>(m_memory.base), m_memory.size, 0, face);
		break;
	}

	if (error) {
	#ifdef FTGL_STDERR_DISPLAY
		fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
		        __LINE__, FT_Errors[error].code, FT_Errors[error].message);
	#endif
		FT_Done_FreeType(*library);
		return false;
	}

	/* Select charmap */
	error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
	if (error) {
	#ifdef FTGL_STDERR_DISPLAY
		fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
		        __LINE__, FT_Errors[error].code, FT_Errors[error].message);
	#endif

		FT_Done_Face(*face);
		FT_Done_FreeType(*library);
		return false;
	}

	/* Set char size */
	error = FT_Set_Char_Size(*face, (int)(size * HRES), 0, DPI * HRES, DPI);

	if (error) {
	#ifdef FTGL_STDERR_DISPLAY
		fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
			__LINE__, FT_Errors[error].code, FT_Errors[error].message);
	#endif
		FT_Done_Face(*face);
		FT_Done_FreeType(*library);
		return false;
	}

	/* Set transform matrix */
	FT_Set_Transform(*face, &matrix, nullptr);

	return true;
}

void ftgl::Font::generateKerning()
{
	FT_Library library;
	FT_Face face;
	FT_UInt glyph_index, prev_index;
	Glyph* glyph, *prev_glyph;
	FT_Vector kerning;


	/* Load font */
	if (!loadFace(m_size, &library, &face))
		return;

	/* For each glyph couple combination, check if kerning is necessary */
	/* Starts at index 1 since 0 is for the special background glyph */
	for (size_t i = 1; i < m_glyphs.size(); ++i)
	{
		glyph = &m_glyphs[i];
		glyph_index = FT_Get_Char_Index(face, glyph->codepoint);
		glyph->kernings.clear();

		//TODO: Improve this code
		for (size_t j = 1; j < m_glyphs.size(); ++j)
		{
			prev_glyph = &m_glyphs[i];
			prev_index = FT_Get_Char_Index(face, prev_glyph->codepoint);
			FT_Get_Kerning(face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning);

			if (kerning.x)
			{
				Kerning k = { prev_glyph->codepoint, kerning.x / (HRESf * HRESf) };
				glyph->kernings.push_back(k);
			}
		}
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

ftgl::Glyph* ftgl::Font::findGlyph(uint32_t ucodepoint)
{
	for (auto&& glyph : m_glyphs)
	{
		// If codepoint is -1, we don't care about outline type or thickness
		if ((glyph.codepoint == ucodepoint) &&
			((ucodepoint == -1) ||
			((glyph.outline_type == m_outline_type) &&
				(glyph.outline_thickness == m_outline_thickness))))
		{
			return &glyph;
		}
	}
	return nullptr;
}





