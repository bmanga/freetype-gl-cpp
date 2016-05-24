#pragma once
#include <cassert>
#include <cstring>
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
		strLength = strlen(str);
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


};
}
