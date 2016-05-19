﻿#include <iostream>
#include "TextureFont.h"
#include "texture-atlas.h"
#include "texture-font.h"
#include <chrono>
#include <random>
#include <algorithm>


std::vector<char> letters(2'000'000);
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
	std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
public:
	ScopedTimer() = default;
	~ScopedTimer()
	{
		auto now = std::chrono::high_resolution_clock::now();
		std::cout << "elapsed :" << std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count() / 1e9;
		std::cout << std::endl;
	}

};

int main()
{
	
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
