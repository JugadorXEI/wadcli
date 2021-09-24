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

#include <iostream>
#include <fstream>
#include "headers/palette.h"

DoomPalette::DoomPalette(std::vector<char> data, bool onlyFirstPalette)
	: numberOfPalettes{ data.size() / (DoomPalette::numIndexesPerPalette * 3) }
{
	std::cout << "palettes found: " << numberOfPalettes << '\n';

	int index{ 0 };
	for (size_t i = 0; i < numberOfPalettes; i++)
	{
		for (size_t j = 0; j < DoomPalette::numIndexesPerPalette; j++)
		{
			for (size_t k = 0; j < 3; k++)
			{
				palettes[i][j][k] = data[++index];
			}
		}
	}

	std::cout << "Done\n"; 
}

DoomPalette::DoomPalette(std::string_view fileName, bool onlyFirstPalette)
{
	std::ifstream playfile { fileName.data(), std::ios_base::binary };

	if (!playfile || playfile.fail())
		return;

	// Getting size.
	playfile.seekg(0, std::ios::end);
	const uint32_t dataSize{ static_cast<uint32_t>(playfile.tellg()) };
	playfile.seekg(0, std::ios::beg);

	// Create and resize vector.
	std::vector<char> binary{};
	binary.resize(dataSize);

	// Put all the data in.
	playfile.read(&binary[0], dataSize);

	DoomPalette(std::move(binary), onlyFirstPalette);
}

int DoomPalette::getNumPalettes() { return (*this).numberOfPalettes; }

std::array<char, 3> DoomPalette::getColorFromPalette(int palette, int colorIndex)
{
	return palettes[palette][colorIndex];
}

std::array<std::array<char, 3>, DoomPalette::numIndexesPerPalette>& DoomPalette::operator[](const unsigned int index)
{
	return palettes[index];
}