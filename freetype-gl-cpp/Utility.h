#pragma once
#include <cassert>
#include <cstring>
#include <string>
#include <iterator>
#include "TextureFont.h"
#include "utf8Utils.h"

namespace ftgl {
class cstring_view
{
private:
	const char* start;
	size_t strLength;

public:
	cstring_view() = default;

	cstring_view(const cstring_view&) = default;

	cstring_view(const char* str) :
		start(str), strLength(strlen(str)) {}

	cstring_view(const char* start, size_t length) :
		start(start), strLength(length) {}

	cstring_view(const char* start, const char* end) :
		start(start), strLength(end - start) {}

	template <size_t N>
	cstring_view(const char(&str)[N]):
		start(str), strLength(N) {}

	cstring_view& operator=(const cstring_view&) = default;

	cstring_view& operator=(const char* str)
	{
		start = str;
		strLength = strlen(str);
		return *this;
	}

	template <size_t N>
	cstring_view& operator=(const char*(&str)[N])
	{
		start = str;
		strLength = N;
		return *this;
	}

	operator const char*() const
	{
		return start;
	}

	size_t size() const
	{
		return strLength;
	}

	size_t length() const
	{
		return strLength;
	}

	char operator[](size_t pos) const
	{
		assert(pos < strLength);
		return *(start + pos);
	}

	const char* findPtr(char c) const
	{
		return static_cast<const char*>(std::memchr(start, c, strLength));
	}

	void assign(const char* str, size_t len)
	{
		start = str;
		strLength = len;
	}

	std::string to_string() const
	{
		return std::string(start, start + strLength);
	}
};

class glyph_iterator :
	public std::iterator<std::forward_iterator_tag, const Font*>
{
private:
	size_t index = 0;
	const char* codepoints = nullptr;
	Font* font = nullptr;

public:
	glyph_iterator() = default;

	glyph_iterator(Font* font, const char* codepoints, size_t index) :
		index(index),
		codepoints(codepoints),
		font(font)
	{
		assert(codepoints);
	}

	glyph_iterator(const glyph_iterator&) = default;

	glyph_iterator& operator++()
	{
		index += utf8_surrogate_len(codepoints + index);
		return *this;
	}

	glyph_iterator operator++(int)
	{
		glyph_iterator previous(*this);
		index += utf8_surrogate_len(codepoints + index);
		return previous;
	}

	const Glyph* operator*() const
	{
		return font->getLoadedGlyph(utf8_to_utf32(codepoints + index));
	}

	const Glyph* operator->() const
	{
		return font->getLoadedGlyph(utf8_to_utf32(codepoints + index));
	}

	bool operator== (const glyph_iterator& itr) const
	{
		assert(itr.codepoints == codepoints && itr.font == font);

		if (itr.index == index) return true;
		return false;
	}

	bool operator!= (const glyph_iterator& itr) const
	{
		return !(*this == itr);
	}
};

class glyph_range
{
private:
	const char* codepoints;
	Font* font;
public:
	glyph_range(Font* font, const char* codepoints) :
		codepoints(codepoints),
		font(font)
	{
		font->loadGlyphs(codepoints);
	}

	glyph_iterator begin() const
	{
		return{ font, codepoints, 0 };
	}

	glyph_iterator end() const
	{
		return{ font, codepoints, utf8_strlen(codepoints) };
	}

	std::vector<const Glyph*> as_vector() const
	{
		std::vector<const Glyph*> glyphs;
		std::copy(begin(), end(), std::back_inserter(glyphs));
		return glyphs;
	}
};

}//namespace ftgl
