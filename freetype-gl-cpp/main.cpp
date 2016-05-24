#include <iostream>
#include "TextureFont.h"
#include "texture-atlas.h"
#include "texture-font.h"
#include <chrono>
#include <random>
#include <algorithm>
#include "VertexBuffer.h"
#include "opengl.h"

std::vector<char> letters(200'0000);
std::vector<wchar_t> letters2(5);

void init()
{
	std::mt19937 rng;
	rng.seed(std::random_device()());
	std::uniform_int_distribution<std::mt19937::result_type> dist(0x21, 120); 

	std::generate(letters.begin(), letters.end(), [&]()
	{
		return char(dist(rng));
	});

	std::generate(letters2.begin(), letters2.end(), [&]()
	{
		return char(dist(rng));
	});
	//letters = { 19, 72, -6, 23, 16 };
	//letters = { 1, 33 };

}

class ScopedTimer
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start = 
		std::chrono::high_resolution_clock::now();
public:
	ScopedTimer() = default;
	~ScopedTimer()
	{
		auto now = std::chrono::high_resolution_clock::now();
		std::cout << "elapsed :" <<
			std::chrono::duration_cast<std::chrono::nanoseconds>(now - start)
			.count() / 1e9;
		std::cout << std::endl;
	}

};

struct st
{
	float x, y, z;
	float s, t;
	float r, g, b, a;
};
int main()
{
	glewInit();

	ftgl::VertexBuffer buffer("vertex:3f,tex_coord:2f,color:4f");
	unsigned indices[] = { 12, 22, 44, 55, 66, 77 };
	st test[] = {
		{ 1.0f, 1.0f, 3.0f, 2.0f, 4.0f, 1.0f, 0.03f, 0.8f, 1.0f },
		{ 1.4f, 1.0f, 3.0f, 2.1f, 11.f, 1.0f, 0.03f, 1.8f, 1.0f }
	};
	buffer.pushBackVertices((char*)&test, 2);

	//buffer.render(GL_LINE);

	init();
	char tmp[] = { '\0', '\0', '\0', '\0', '\0', '\0' };
	const char* str = tmp;
	std::cout << "\nrun 1: c++ first, c second\n";
	using namespace ftgl;

	//c++
	{

		ScopedTimer t;
		TextureAtlas atlas(512, 512, 1);
		Font font(&atlas, Font::File{ 32, "Xanadu.ttf" });
		//font.loadGlyphs(u8"لأَبْجَدِيَّة العَرَبِيَّة");
		size_t var = 0;
		for (auto what : letters) {
			tmp[0] = what;
			auto* glyph = font.getGlyph(tmp);
			var += glyph->height;

		}

		std::cout << var << std::endl;
	}
	
	//c
	{
		
		ScopedTimer t;
		auto* atlas = texture_atlas_new(512, 512, 1);
		auto* font = texture_font_new_from_file(atlas, 32, "Xanadu.ttf");
		size_t var = 0;
		for (auto what : letters)
		{
			tmp[0] = what;
			auto* glyph = texture_font_get_glyph(font, str);
			var += glyph->height;

		}
		texture_atlas_delete(atlas);
		texture_font_delete(font);

		std::cout << var << std::endl;
	}


	init();
	std::cout << "\nrun 2: c first, c second++\n";

	//c
	{

		ScopedTimer t;
		auto* atlas = texture_atlas_new(512, 512, 1);
		auto* font = texture_font_new_from_file(atlas, 32, "Xanadu.ttf");
		size_t var = 0;
		for (auto what : letters)
		{
			tmp[0] = what;
			auto* glyph = texture_font_get_glyph(font, str);
			var += glyph->height;

		}
		texture_atlas_delete(atlas);
		texture_font_delete(font);

		std::cout << var << std::endl;
	}

	//c++
	{

		ScopedTimer t;
		TextureAtlas atlas(512, 512, 1);
		Font font(&atlas, Font::File{ 32, "Xanadu.ttf" });

		size_t var = 0;
		for (auto what : letters) {
			tmp[0] = what;
			auto* glyph = font.getGlyph(tmp);
			var += glyph->height;

		}

		std::cout << var << std::endl;
	}



	system("PAUSE");
}
