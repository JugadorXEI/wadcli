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

#ifndef JUG_WADFORMAT_H
#define JUG_WADFORMAT_H

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>

enum WadType
{
	INVALID = -1, 	// This isn't even a WAD!
	IWAD 	= 0, 	// Internal WAD
	PWAD 	= 1, 	// Patch WAD
	ZWAD	= 2, 	// ZIP WAD
	SDLL	= 3,	// SDLL (SRB2 easter egg WADs)
	CUSTOM	= 100	// Whatever else (SDLL and so)
};

struct WadFile
{
	uint32_t dataOffset; // this is for informative uses only
	uint32_t dataSize;
	std::string name;
	std::vector<char> binaryData;
};

class WadFormat
{
public:
	static constexpr int minSizeForCompression{ 1024 };
	static constexpr int fileNameLength{ 8 };

	WadFormat();
	WadFormat(std::string_view fileName);
	WadFormat(std::string_view fileName, WadType wadType);

	uint32_t 	getNumFiles();
	uint32_t 	getFATOffset();
	WadType 	getWADType();
	std::string_view getWADTypeToChar();
	std::string&	getWADName();
	std::vector<WadFile>& getWADLumpList();

	WadFile& getFileFromIndex(const unsigned int index);
	WadFile& operator[](const unsigned int index);

	bool exportWAD(std::string_view fileName);
	bool importWAD(std::string_view fileName);
	
	bool compressWAD();
	void compressFile(WadFile& file);
	bool decompressWAD(WadType newType = WadType::PWAD);
	void decompressFile(WadFile& file);

	bool addFileToWAD(std::string_view filename, std::string_view newname = "", bool override = false);
	void addFileToWAD(WadFile& file);
	void removeFileByIndex(const unsigned int index);
	bool removeFileByName(std::string_view filename);
	
	std::pair<bool, std::optional<std::string>>
		extractLump(WadFile& file, bool noExtension = false, bool noOverride = false, std::string_view path = "");

	void createMarkers(std::string_view markerName);

	bool swapLumpPosByName(std::string_view name1, std::string_view name2);
	void swapLumpPosByIndex(unsigned int index1, unsigned int index2);

	bool moveLumpPosByName(std::string_view name1, int position, bool relative);
	bool moveLumpPosByIndex(unsigned int index, int position, bool relative);

	static std::string_view determineFormatFromFileName(std::string_view fileName);
	static void trimStringToMarkerCharacters(std::string& markerName);

private:
	WadType 	wadType;
	std::string wadName;
	uint32_t 	wadNumFiles;
	uint32_t 	wadOffFAT;
	std::vector<WadFile> wadFiles;

	void setWADType(WadType newType);
};

#endif