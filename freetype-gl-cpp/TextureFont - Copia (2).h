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
#pragma once

#include <stdlib.h>
#include <cstdint>

#include "TextureAtlas.h"

namespace ftgl
{
	/**
	 * @file   texture-font.h
	 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
	 *
	 * @defgroup texture-font Texture font
	 *
	 * Texture font.
	 *
	 * Example Usage:
	 * @code
	 * #include "texture-font.h"
	 *
	 * int main( int arrgc, char *argv[] )
	 * {
	 *   return 0;
	 * }
	 * @endcode
	 *
	 * @{
	 */


	/**
	 * A structure that hold a kerning value relatively to a Unicode
	 * codepoint.
	 *
	 * This structure cannot be used alone since the (necessary) right
	 * Unicode codepoint is implicitely held by the owner of this structure.
	 */
	struct Kerning
	{
		/**
		 * Left Unicode codepoint in the kern pair in UTF-32 LE encoding.
		 */
		uint32_t codepoint;

		/**
		 * Kerning value (in fractional pixels).
		 */
		float kerning;
	};


	/*
	 * Glyph metrics:
	 * --------------
	 *
	 *                       xmin                     xmax
	 *                        |                         |
	 *                        |<-------- width -------->|
	 *                        |                         |
	 *              |         +-------------------------+----------------- ymax
	 *              |         |    ggggggggg   ggggg    |     ^        ^
	 *              |         |   g:::::::::ggg::::g    |     |        |
	 *              |         |  g:::::::::::::::::g    |     |        |
	 *              |         | g::::::ggggg::::::gg    |     |        |
	 *              |         | g:::::g     g:::::g     |     |        |
	 *    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
	 *              |         | g:::::g     g:::::g     |     |        |
	 *              |         | g::::::g    g:::::g     |     |        |
	 *              |         | g:::::::ggggg:::::g     |     |        |
	 *              |         |  g::::::::::::::::g     |     |      height
	 *              |         |   gg::::::::::::::g     |     |        |
	 *  baseline ---*---------|---- gggggggg::::::g-----*--------      |
	 *            / |         |             g:::::g     |              |
	 *     origin   |         | gggggg      g:::::g     |              |
	 *              |         | g:::::gg   gg:::::g     |              |
	 *              |         |  g::::::ggg:::::::g     |              |
	 *              |         |   gg:::::::::::::g      |              |
	 *              |         |     ggg::::::ggg        |              |
	 *              |         |         gggggg          |              v
	 *              |         +-------------------------+----------------- ymin
	 *              |                                   |
	 *              |------------- advance_x ---------->|
	 */

	/**
	 * A structure that describe a glyph.
	 */

	struct Glyph
	{
		/**
		 * Unicode codepoint this glyph represents in UTF-32 LE encoding.
		 */
		uint32_t codepoint = -1;

		/**
		* Glyph outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
		*/
		int outline_type = 0;

		/**
		* Glyph outline thickness
		*/
		float outline_thickness = 0.0f;

		/**
		 * Glyph's width in pixels.
		 */
		size_t width = 0;

		/**
		 * Glyph's m_height in pixels.
		 */
		size_t height = 0;

		/**
		 * Glyph's left bearing expressed in integer pixels.
		 */
		int offset_x = 0;

		/**
		 * Glyphs's top bearing expressed in integer pixels.
		 *
		 * Remember that this is the distance from the baseline to the top-most
		 * glyph scanline, upwards y coordinates being positive.
		 */
		int offset_y = 0;

		/**
		 * For horizontal text layouts, this is the horizontal distance (in
		 * fractional pixels) used to increment the pen position when the glyph is
		 * drawn as part of a string of text.
		 */
		float advance_x = 0;

		/**
		 * For vertical text layouts, this is the vertical distance (in fractional
		 * pixels) used to increment the pen position when the glyph is drawn as
		 * part of a string of text.
		 */
		float advance_y = 0;

		/**
		 * First normalized texture coordinate (x) of top-left corner
		 */
		float s0 = 0.0f;

		/**
		 * Second normalized texture coordinate (y) of top-left corner
		 */
		float t0 = 0.0f;

		/**
		 * First normalized texture coordinate (x) of bottom-right corner
		 */
		float s1 = 0.0f;

		/**
		 * Second normalized texture coordinate (y) of bottom-right corner
		 */
		float t1 = 0.0f;

		/**
		 * A vector of kerning pairs relative to this glyph.
		 */
		std::vector<Kerning> kernings;



	public:
		float getKerning(const char* codepoint) const;
	};

	//Forward declarations of freetype structs
	typedef struct FT_LibraryRec_* FT_Library;
	typedef struct FT_FaceRec_* FT_Face;


	/**
	 *  Texture font structure.
	 */
	class Font
	{
	private:
		struct QuickGlyphData
		{
			uint32_t codepoint;
			uint32_t  outline_thickness_type;
		};
		static_assert(sizeof QuickGlyphData == 8, "");

		/**
		 * Vector of glyphs contained in this font.
		 */
		std::vector<Glyph> m_glyphs;

		/**
		 * This vector is used for quicker iteration in findGlyph
		 * It shoul dbe in sync with m_glyphs
		 */

		std::vector<QuickGlyphData> m_glyph_find;

		/**
		 * Atlas structure to store glyphs data.
		 */
		TextureAtlas* m_atlas;

		/**
		 * font location
		 */
		enum Location
		{
			TEXTURE_FONT_FILE = 0,
			TEXTURE_FONT_MEMORY,
		} m_location;

		union
		{
			/**
			 * Font filename, for when location == TEXTURE_FONT_FILE
			 */
			std::string m_filename;

			/**
			 * Font memory address, for when location == TEXTURE_FONT_MEMORY
			 */
			struct
			{
				const void* base;
				size_t size;
			} m_memory;
		};

		/**
		 * Font size
		 */
		float m_size;


		/**
		 * Outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
		 */
		int m_outline_type = 0;

		/**
		 * Outline thickness
		 */
		float m_outline_thickness = 0.0f;

		uint32_t m_outline_thickness_type = 0;
		/**
		 * LCD filter weights
		 */
		// FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
		// FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
		unsigned char m_lcd_weights[5] = {0x10, 0x40, 0x70, 0x40, 0x10};



		/**
		 * This field is simply used to compute a default line spacing (i.e., the
		 * baseline-to-baseline distance) when writing text with this font. Note
		 * that it usually is larger than the sum of the ascender and descender
		 * taken as absolute values. There is also no guarantee that no glyphs
		 * extend above or below subsequent baselines when using this distance.
		 */
		float m_height = 0;

		/**
		 * This field is the distance that must be placed between two lines of
		 * text. The baseline-to-baseline distance should be computed as:
		 * ascender - descender + linegap
		 */
		float m_linegap;

		/**
		 * The ascender is the vertical distance from the horizontal baseline to
		 * the highest 'character' coordinate in a font face. Unfortunately, font
		 * formats define the ascender differently. For some, it represents the
		 * ascent of all capital latin characters (without accents), for others it
		 * is the ascent of the highest accented character, and finally, other
		 * formats define it as being equal to bbox.yMax.
		 */
		float m_ascender = 0;

		/**
		 * The descender is the vertical distance from the horizontal baseline to
		 * the lowest 'character' coordinate in a font face. Unfortunately, font
		 * formats define the descender differently. For some, it represents the
		 * descent of all capital latin characters (without accents), for others it
		 * is the ascent of the lowest accented character, and finally, other
		 * formats define it as being equal to bbox.yMin. This field is negative
		 * for values below the baseline.
		 */
		float m_descender = 0;

		/**
		 * The position of the underline line for this face. It is the center of
		 * the underlining stem. Only relevant for scalable formats.
		 */
		float m_underline_position;

		/**
		 * The thickness of the underline for this face. Only relevant for scalable
		 * formats.
		 */
		float m_underline_thickness;

		/**
		* Whether to use autohint when rendering font
		*/
		bool m_hinting = true;

		/**
		* Whether to use kerning if available
		*/
		bool m_kerning = true;

		/**
		* Whether to use our own lcd filter.
		*/
		bool m_filtering = true;

		bool m_success = false;

	public:
		struct File
		{
			float pt_size;
			const char* filename;
		};

		struct Memory
		{
			float pt_size;
			const char* memory_base;
			size_t memory_size;
		};

		explicit Font(TextureAtlas* atlas, File file);
		explicit Font(TextureAtlas* atlas, Memory memory);

		~Font()
		{
		}

		const Glyph* getGlyph(const char* codepoint);
		size_t loadGlyphs(const char* codepoints);
		bool loadGlyph(uint32_t ucodepoint);

		operator bool() const
		{
			return m_success;
		}

	private:
		bool loadFace(float size, FT_Library* library, FT_Face* face);
		void generateKerning();
		Glyph* findGlyph(uint32_t ucodepoint);
		bool init();
	};


	/**
	 * This function creates a new texture font from given filename and size.  The
	 * texture atlas is used to store glyph on demand. Note the depth of the atlas
	 * will determine if the font is rendered as alpha channel only (depth = 1) or
	 * RGB (depth = 3) that correspond to subpixel rendering (if available on your
	 * freetype implementation).
	 *
	 * @param atlas     A texture atlas
	 * @param pt_size   Size of font to be created (in points)
	 * @param filename  A font filename
	 *
	 * @return A new empty font (no glyph inside yet)
	 *
	 */

	//texture_font_t *
	//texture_font_new_from_file( texture_atlas_t * atlas,
	//                            const float pt_size,
	//                            const char * filename );


	/**
	 * This function creates a new texture font from a memory location and size.
	 * The texture atlas is used to store glyph on demand. Note the depth of the
	 * atlas will determine if the font is rendered as alpha channel only
	 * (depth = 1) or RGB (depth = 3) that correspond to subpixel rendering (if
	 * available on your freetype implementation).
	 *
	 * @param atlas       A texture atlas
	 * @param pt_size     Size of font to be created (in points)
	 * @param memory_base Start of the font file in memory
	 * @param memory_size Size of the font file memory region, in bytes
	 *
	 * @return A new empty font (no glyph inside yet)
	 *
	 */

	//texture_font_t *
	//texture_font_new_from_memory( texture_atlas_t *atlas,
	//                              float pt_size,
	//                              const void *memory_base,
	//                              size_t memory_size );

	/**
	 * Delete a texture font. Note that this does not delete the glyph from the
	 * texture atlas.
	 *
	 * @param self a valid texture font
	 */

	//void
	//texture_font_delete( texture_font_t * self );


	/**
	 * Request a new glyph from the font. If it has not been created yet, it will
	 * be.
	 *
	 * @param self      A valid texture font
	 * @param codepoint Character codepoint to be loaded in UTF-8 encoding.
	 *
	 * @return A pointer on the new glyph or 0 if the texture atlas is not big
	 *         enough
	 *
	 */
	/*texture_glyph_t *
	texture_font_get_glyph( texture_font_t * self,
	                        const char * codepoint );
  */

	/**
	 * Request the loading of several glyphs at once.
	 *
	 * @param self       A valid texture font
	 * @param codepoints Character codepoints to be loaded in UTF-8 encoding. May
	 *                   contain duplicates.
	 *
	 * @return Number of missed glyph if the texture is not big enough to hold
	 *         every glyphs.
	 */
	/*size_t
	texture_font_load_glyphs( texture_font_t * self,
	                          const char * codepoints );
  */
	/**
	 * Get the kerning between two horizontal glyphs.
	 *
	 * @param self      A valid texture glyph
	 * @param codepoint Character codepoint of the peceding character in UTF-8 encoding.
	 *
	 * @return x kerning value
	 */
	//float
	//texture_glyph_get_kerning( const texture_glyph_t * self,
	//                           const char * codepoint );


	/**
	 * Creates a new empty glyph
	 *
	 * @return a new empty glyph (not valid)
	 */
	//texture_glyph_t *
	//texture_glyph_new( void );

	/** @} */
}

