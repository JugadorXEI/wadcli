/*
Copyright (c) 2021, JugadorXEI

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef JUG_PALETTE_H
#define JUG_PALETTE_H

#include <vector>
#include <array>
#include <string_view>

class DoomPalette
{
public:
	static const unsigned int numIndexesPerPalette{ 255 };
	static const unsigned int paletteUpperCap{ 14 };
	static const unsigned int paletteLowerCap{ 0 };

	DoomPalette(std::vector<char> data, bool onlyFirstPalette = false);
	DoomPalette(std::string_view fileName, bool onlyFirstPalette = false);

	int getNumPalettes();
	std::array<char, 3> getColorFromPalette(int palette, int colorIndex);
	std::array<std::array<char, 3>, numIndexesPerPalette>& operator[](const unsigned int index);
	

private:
	int numberOfPalettes;
	/*
		Array that:
			Containers 14 palettes
				Each palette contains 255 indexes
					Each index contains 3 chars of color
	*/
	std::array<
		std::array<
			std::array<char, 3>, numIndexesPerPalette>, paletteUpperCap> palettes;
};

#endif