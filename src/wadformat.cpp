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

#include <fstream>
#include <cstring>
#include <vector>
#include <iostream>
#include <functional>
#include <liblzf/lzf.h>
#include "headers/wadformat.h"

WadFormat::WadFormat(std::string_view fileName)
	: wadType{ WadType::INVALID }, wadName{ fileName }, wadNumFiles{ 0 }, wadOffFAT{ 12 }, wadFiles{ 0 }
{
	// empty.
}

WadFormat::WadFormat()
	: wadType{ WadType::PWAD }, wadName{ "new.wad" }, wadNumFiles{ 0 }, wadOffFAT{ 12 }, wadFiles{ 0 }
{
	// empty.
}

WadFormat::WadFormat(std::string_view name, WadType type)
	: wadType{ type }, wadName{ name }, wadNumFiles{ 0 }, wadOffFAT{ 12 }, wadFiles{ 0 }
{
	// empty.
}

bool WadFormat::exportWAD(std::string_view fileName)
{
	if (WadFormat::getWADType() == WadType::INVALID)
	{
		std::cerr << "exportWAD: Trying to export an invalid WAD. Quitting early." << '\n';
		return false;
	}

	std::ofstream newWadStream{ fileName.data() };

	// Type of wad, number of files, location of FAT.
	newWadStream.write(WadFormat::getWADTypeToChar().data(), sizeof(char) * 4);

	uint32_t numFiles{ WadFormat::getNumFiles() }; 
	newWadStream.write(reinterpret_cast<const char*>(&numFiles), sizeof(numFiles));

	const long long int FATOffsetPos{ newWadStream.tellp() };
	newWadStream.write("0000", sizeof(char) * 4);

	// TIME TO DUMP ALL THE DATA!
	std::vector<uint32_t> dataOffsets{};
	dataOffsets.resize(numFiles);

	for (size_t i = 0; i < numFiles; ++i)
	{
		dataOffsets[i] = newWadStream.tellp();
		newWadStream.write(&(*this)[i].binaryData[0], (*this)[i].dataSize);
	}
	
	// We dumped everything, now let's set the FAT offset.
	const uint32_t FATOffsetStart{ static_cast<uint32_t>(newWadStream.tellp()) };
	newWadStream.seekp(FATOffsetPos);
	newWadStream.write(reinterpret_cast<const char*>(&FATOffsetStart), sizeof(FATOffsetStart));
	newWadStream.seekp(FATOffsetStart);

	// Time to write the FAT.
	for (size_t i = 0; i < numFiles; ++i)
	{
		// std::cout << "inside " << i << "\n";
		newWadStream.write(reinterpret_cast<const char*>(&dataOffsets[i]), sizeof(dataOffsets[i]));
		newWadStream.write(reinterpret_cast<const char*>(&(*this)[i].dataSize), sizeof((*this)[i].dataSize));

		size_t stringLength{ (*this)[i].name.size() };
		newWadStream.write((*this)[i].name.c_str(),
			static_cast<long long int>(stringLength > fileNameLength ? fileNameLength : stringLength) );

		// Write empty data if string is less than 8.
		if (stringLength < fileNameLength)
		{
			// This looks so silly...
			stringLength = fileNameLength - stringLength;
			newWadStream.write("\0\0\0\0\0\0\0\0", static_cast<long long int>(stringLength));
		}

		if constexpr (DEBUG) 
			std::cout << (*this)[i].name.c_str() << '\n';
	}
	
	if constexpr (DEBUG) 
		std::cout << "Done exporting " << fileName << ".\n";

	newWadStream.close();
	return true;
}

bool WadFormat::importWAD(std::string_view fileName)
{
	std::ifstream wadBinary{ fileName.data(), std::ios_base::binary };

	if (!wadBinary || wadBinary.fail())
		return false;

	// The buffer.
	const int length{ 4 };
	std::string buffer{};
	buffer.resize(length);

	// Get type of wad.
	wadBinary.read(buffer.data(), length);
	wadType = 	 buffer == "IWAD" ?	WadType::IWAD :
				(buffer == "PWAD" ? WadType::PWAD :
				(buffer == "ZWAD" ? WadType::ZWAD :
				(buffer == "SDLL" ?	WadType::SDLL : 
									WadType::INVALID)));
									
	if (DEBUG) std::cout << buffer << '\n';

	// Quit early.
	if (wadType == WadType::INVALID)
		return false;
	
	// Get number of files and the offset to the FAT
	wadBinary.read(buffer.data(), length);
	std::memcpy(&wadNumFiles, buffer.data(), sizeof(uint32_t));

	wadBinary.read(buffer.data(), length);
	std::memcpy(&wadOffFAT, buffer.data(), sizeof(uint32_t));

	// Ok, let's get to the file list.
	wadBinary.seekg(wadOffFAT);
	wadFiles.reserve(wadNumFiles * sizeof(WadFile));

	for (size_t i = 0; i < wadNumFiles; ++i)
	{
		uint32_t fileDataOffset{ 0 };
		uint32_t fileDataSize{ 0 };

		// Where's the data at?
		wadBinary.read(buffer.data(), length);
		std::memcpy(&fileDataOffset, buffer.data(), sizeof(uint32_t));

		// How big's the data?
		wadBinary.read(buffer.data(), length);
		std::memcpy(&fileDataSize, buffer.data(), sizeof(uint32_t));

		// Name of the file.
		const int nameLength{ fileNameLength };
		char* nameBuffer = new char[nameLength + 1]; // 8 + null terminator
		nameBuffer[nameLength] = '\0';
		wadBinary.read(nameBuffer, nameLength);

		const long long int currentPositionInFile{ wadBinary.tellg() };

		// Binary data of the file.
		std::vector<char> binary{};
		binary.resize(fileDataSize);
		wadBinary.seekg(fileDataOffset);
		wadBinary.read(&binary[0], fileDataSize);

		// We're done, let's put the cursor where it was before.
		wadBinary.seekg(currentPositionInFile);
		
		//std::cout << nameBuffer << '\n';
		wadFiles.push_back({fileDataOffset, fileDataSize, std::string{nameBuffer, nameLength}, std::move(binary)});

		delete[] nameBuffer;
	}

	// Ok, we're done.
	wadBinary.close();

	return true;
}

WadType WadFormat::getWADType() 	{ return wadType; }
std::string_view WadFormat::getWADTypeToChar()
{
	return (WadFormat::getWADType() == WadType::IWAD ? std::string_view{"IWAD"} :
			(WadFormat::getWADType() == WadType::PWAD ? std::string_view{"PWAD"} :
			(WadFormat::getWADType() == WadType::ZWAD ? std::string_view{"ZWAD"} : 
			(WadFormat::getWADType() == WadType::SDLL ? std::string_view{"SDLL"} : std::string_view{"INVALID"}))));	
}

void WadFormat::setWADType(WadType newType) { (*this).wadType = newType; }

void WadFormat::compressFile(WadFile& file)
{
	uint32_t dataSizeForThisFile{ file.dataSize };

	std::vector<char> compressedBinary{};
	compressedBinary.resize(dataSizeForThisFile + 4);

	// wadzip does not uncompress below 1024 B but I'll make this changeable.

	// Size reported in FAT is compressed,
	// first four bytes of the lump is uncompressed.

	// Don't bother compressing files less than one KB.
	if (dataSizeForThisFile < WadFormat::minSizeForCompression)
	{
		for (size_t i = 0; i < 4; i++) // First four bytes
		{
			// std::cout << "byte " << i << '\n'; 
			compressedBinary[i] = 0;
		}

		compressedBinary.insert(compressedBinary.begin() + 4,
			file.binaryData.begin(), file.binaryData.end());

		file.dataSize 	+= 4;
		file.binaryData = std::move(compressedBinary);
	}
	else
	{
		unsigned int compressedSize = lzf_compress(&file.binaryData[0], file.dataSize,
			(compressedBinary.begin() + 4).base(), file.dataSize - 1);

		if (compressedSize == 0 && errno == 0) // buffer too small.
		{
			for (size_t i = 0; i < 4; i++)
				compressedBinary[i] = 0;

			compressedBinary.insert(compressedBinary.begin() + 4,
				file.binaryData.begin(), file.binaryData.end());

			file.dataSize	+= 4;
		}
		else
		{
			for (size_t i = 24; i <= 24; i -= 8)
			{
				// std::cout << i << '\n';
				compressedBinary[i / 8] = dataSizeForThisFile >> i;
			}

			// + 4 for the data bytes that indicate uncompressed size
			file.dataSize 	= compressedSize + 4; 
		}

		file.binaryData = std::move(compressedBinary);
	}
}

bool WadFormat::compressWAD()
{
	//	1. (done) The header id at the very beginning says "ZWAD"
	//	instead of PWAD or IWAD.
	if ((*this).getWADType() == WadType::ZWAD)
	{
		std::cerr << "Can't compress an already compressed ZWAD. Quitting early.\n";
		return false;
	}

	// We're pretty much assuming here that every file is uncompressed.
	for (size_t i = 0; i < (*this).getNumFiles(); ++i)
	{
		WadFile& file = (*this)[i];
		(*this).compressFile(file);
	}

	(*this).setWADType(WadType::ZWAD);
	return true;
}

void WadFormat::decompressFile(WadFile& file)
{
	uint32_t uncompressedSize{ 0 };
	std::memcpy(&uncompressedSize, &file.binaryData[0], sizeof(uint32_t));

	// std::cout << "Size: " << uncompressedSize << '\n';

	if (uncompressedSize == 0)
	{
		/*
			2. The first four bytes of each lump are (little
			endian) the uncompressed size. If this is zero,
			the lump is not compressed, and you can subtract
			four from the size given in the wadfile directory.
		*/
		file.binaryData.insert(file.binaryData.begin(),
			file.binaryData.begin() + 4, file.binaryData.end());
		file.dataSize -= 4;
	}
	else
	{
		/*
			3. Unless those first four bytes give a value of
			0, the lump is compressed with liblzf after that.
		*/
		std::vector<char> uncompressedBinary{};
		uncompressedBinary.resize(uncompressedSize);

		lzf_decompress(&file.binaryData[4], file.dataSize,
			uncompressedBinary.begin().base(), uncompressedSize);

		file.dataSize 	= uncompressedSize;
		file.binaryData = std::move(uncompressedBinary);
	}
}

bool WadFormat::decompressWAD(WadType newType)
{
	if ((*this).getWADType() != WadType::ZWAD)
	{
		std::cerr << "Can't decompress a non-ZWAD. Quitting early.\n";
		return false;
	}

	for (size_t i = 0; i < (*this).getNumFiles(); ++i)
	{
		WadFile& file = (*this)[i];
		(*this).decompressFile(file);
	}

	(*this).setWADType(newType);
	return true;
}

bool WadFormat::addFileToWAD(std::string_view filename, std::string_view newname, bool override)
{
	std::ifstream newFile{ filename.data(), std::ios_base::binary };
	if (newFile.fail())
		return false;

	// Getting size for binary vector.
	newFile.seekg(0, std::ios::end);
	const uint32_t dataSize{ static_cast<uint32_t>(newFile.tellg()) };
	newFile.seekg(0, std::ios::beg);

	// Creating and resizing vector
	std::vector<char> binary{};
	binary.resize(dataSize);

	// Dunking all of the info in.
	newFile.read(&binary[0], dataSize);

	// Get offset.
	WadFile& lastFile{ (*this)[(*this).wadNumFiles] };
	const uint32_t dataOffset{ lastFile.dataOffset + lastFile.dataSize };

	// Do we care about the name?
	std::string inputname;
	inputname.resize(9);

	if (newname.empty())
	{
		// Remove extension if exists.
		size_t lastindex{ filename.find_last_of('.') };
		if (lastindex != std::string::npos)
			filename = filename.substr(0, lastindex);

		if (filename.size() > fileNameLength)
		{
			// WAD file names can only be 8 or less.
			// Input user for a new name.
			// If nothing, just crop the current name to 8.

			std::cout << "Input a new name for " << filename << ": ";
			std::cin >> inputname;

			if (inputname.empty())
			{
				std::cout << "Ignored. Cropping file name to 8 characters.\n";
				inputname = filename;
				inputname = inputname.substr(0, 7);
			}
		}
		else
			inputname = filename;
	}
	else
		inputname = newname;
	
	if constexpr (DEBUG)
	{
		std::cout << "string: " 	<< inputname << '\n';
		std::cout << "size: " 		<< dataSize << '\n';
		std::cout << "inputname: " 	<< inputname << '\n';
		std::cout << "wad size: " 	<< wadFiles.size() << '\n';
	}

	unsigned int addIndex{ (*this).wadNumFiles };

	if (override)
	{
		// Find a file with the same name as the file we're adding
		for (size_t i = 0; i < (*this).wadNumFiles; i++)
		{
			if (inputname == (*this)[i].name)
			{
				// Found it, let's replace it.
				addIndex = i;
				break;
			}
		}
	}

	// Add the new file in...
	if (!override)
	{
		// +1 files.
		(*this).wadNumFiles++;
		wadFiles.resize((*this).wadNumFiles);
	}

	wadFiles[addIndex] = { dataOffset, dataSize, inputname, std::move(binary) };

	// Compress if this is a ZWAD.
	if ((*this).getWADType() == WadType::ZWAD)
		(*this).compressFile((*this)[addIndex]);

	return true;
}

void WadFormat::addFileToWAD(WadFile& newFile)
{
	uint32_t sizeOffset{ 0 };
	for (WadFile& file : (*this).wadFiles)
		sizeOffset += file.dataSize;

	(*this).wadFiles.push_back({sizeOffset, newFile.dataSize, newFile.name, std::move(newFile.binaryData)});

	(*this).wadNumFiles++;
}

void WadFormat::removeFileByIndex(const unsigned int index)
{
	uint32_t deletedFileSize{ (*this)[index].dataSize };
	(*this).wadFiles.erase((*this).wadFiles.begin() + index);
	(*this).wadNumFiles--;

	// Set offset to the files preceeding it now that it does not exist.
	for (size_t i = index; i < (*this).getNumFiles(); i++)
		(*this)[index].dataOffset -= deletedFileSize;
}

bool WadFormat::removeFileByName(std::string_view filename)
{
	size_t i{ 0 };
	bool success{ false };

	for (WadFile& file : wadFiles)
	{
		if (filename == file.name.c_str())
		{
			success = true;
			(*this).removeFileByIndex(i);
		}
		++i;
	}

	return success;
}

void WadFormat::createMarkers(std::string_view markerName)
{
	uint32_t sizeOffset{ 0 };
	for (WadFile& file : (*this).wadFiles)
		sizeOffset += file.dataSize;
	
	std::string markerNewNames{ markerName };
	markerNewNames += "_START";

	(*this).wadFiles.push_back({sizeOffset, 0, markerNewNames, std::vector<char>{}});

	markerNewNames = markerName;
	markerNewNames += "_END";

	(*this).wadFiles.push_back({sizeOffset, 0, markerNewNames, std::vector<char>{}});

	(*this).wadNumFiles += 2;
}

std::string_view WadFormat::determineFormatFromFileName(std::string_view fileName)
{
	if (fileName.substr(0, 4) == "SOC_" ||
		fileName.substr(0, 7) == "MAINCFG" ||
		fileName == "OBJCTCFG")
		return ".soc";
	else if (fileName.substr(0, 4) == "LUA_")
		return ".lua";
	/*
	if (fileName.substr(2) == "DS" || fileName.substr(2) == "O_")
	{
		// These sounds could be anything, not just .ogg,
		// and I'd have no good solution to figure these out...
	}
	*/
	else if (fileName.substr(0, 2) == "D_")
		return ".midi";

	return ".lmp";
}

void WadFormat::trimStringToMarkerCharacters(std::string& markerName)
{
	bool foundUnderscore{ false };
	size_t whereIsUnderscore{ 0 };
	for (size_t i = 0; i < markerName.size(); ++i)
	{
		if (markerName[i] == '_')
		{
			foundUnderscore = true;
			whereIsUnderscore = i;
			break;
		}
	}

	if (foundUnderscore)
	{
		markerName = markerName.substr(0, whereIsUnderscore);
		if (markerName.size() > 2)
			// Still? Ok, nuclear option.
			markerName = markerName.substr(0, 2);
	}
	else
		markerName = markerName.substr(0, 2);
}

bool WadFormat::extractLump(WadFile& file, bool noExtension, std::string_view path)
{
	// We should give these an extension.
	std::string filename{ file.name };
	
	if (!noExtension)
	{
		for (size_t i = 0; i < filename.size(); i++)
		{
			if (filename[i] == '\0')
			{
				filename.erase(i, filename.size());
				break;
			}
		}
		
		// Let's add the extension.
		filename.append(determineFormatFromFileName(file.name).data());
	}

	if (!path.empty())
	{
		filename = path.data() + filename;
		if constexpr (DEBUG) 
			std::cout << filename << '\n';
	}

	std::ofstream newFile{ filename, std::ios_base::binary };
	if (newFile.fail())
		return false;

	std::vector<char>& binary{ file.binaryData };
	uint32_t &size{ file.dataSize };
	size_t startingIndex{ 0 };

	if ((*this).getWADType() == ZWAD)
	{
		if constexpr (DEBUG)
			std::cout << "decompressing this before we do anything...\n";

		uint32_t uncompressedSize{ 0 };
		std::memcpy(&uncompressedSize, &file.binaryData[0], sizeof(uint32_t));

		if (uncompressedSize == 0)
		{
			// Start at 4, 4 less bytes when we have a
			// decompressed lump in a compressed WAD.
			startingIndex = 4;
		}
		else
		{
			std::vector<char> uncompressedBinary{};
			uncompressedBinary.resize(uncompressedSize);

			lzf_decompress(&file.binaryData[4], file.dataSize,
				uncompressedBinary.begin().base(), uncompressedSize);

			binary = uncompressedBinary;
			size = uncompressedSize;
		}
	}

	newFile.write(&binary[startingIndex], size - startingIndex);

	newFile.close();
	return true;
}

bool WadFormat::swapLumpPosByName(std::string_view name1, std::string_view name2)
{
	int index1{ -1 };
	int index2{ -1 };

	for (size_t i = 0; i < (*this).getNumFiles(); i++)
	{
		if (strcmp((*this)[i].name.c_str(), name1.data()) == 0)
			index1 = i;
		else if (strcmp((*this)[i].name.c_str(), name2.data()) == 0)
			index2 = i;

		if (index1 != -1 && index2 != -1)
			break;
	}
	
	if (index1 == -1 || index2 == -1)
		return false;

	swapLumpPosByIndex(static_cast<unsigned int>(index1),
		static_cast<unsigned int>(index2));

	return true;
}

void WadFormat::swapLumpPosByIndex(unsigned int index1, unsigned int index2)
{
	WadFile temp{ std::move((*this)[index2]) };
	(*this)[index2] = (*this)[index1];
	(*this)[index1] =  std::move(temp);
}

bool WadFormat::moveLumpPosByName(std::string_view name1, int position, bool relative)
{
	int index{ -1 };
	for (size_t i = 0; i < (*this).getNumFiles(); i++)
	{
		if (strcmp((*this)[i].name.c_str(), name1.data()) == 0)
		{
			index = i;
			break;
		}
	}

	if (index == -1) // couldn't find it
		return false;

	return moveLumpPosByIndex(static_cast<unsigned int>(index), position, relative);
}

bool WadFormat::moveLumpPosByIndex(unsigned int index, int position, bool relative)
{
	int finalIndex = relative ? index + position : position;
	// check if we're not going oob.
	if (static_cast<unsigned int>(finalIndex) > ((*this).getNumFiles() - 1) || finalIndex < 0)
		return false;

	std::function doadd 	{ [](int num) -> int { return ++num; } };
	std::function dominus 	{ [](int num) -> int { return --num; } };

	std::function ishigher	{ [](int num1, int num2) -> bool { return num1 > num2; } };
	std::function islower 	{ [](int num1, int num2) -> bool { return num1 < num2; } };

	// if index is higher, we wanna go up
	std::function<int(int)>& 		arithmetic 	= dominus;
	std::function<bool(int, int)>& 	condition 	= ishigher;

	if (index < static_cast<unsigned int>(finalIndex)) // if index is lower, we wanna go down
	{
		arithmetic	= doadd;
		condition	= islower;
	}

	for (int i = index; condition(i, finalIndex); i = arithmetic(i))
	{
		int newIndex = arithmetic(i);

		if constexpr (DEBUG) 
			std::cout << "i: " << i << " - " << "newIndex: " << newIndex << '\n';

		(*this).swapLumpPosByIndex(i, newIndex);
	}

	return true;
}

uint32_t WadFormat::getNumFiles() 	{ return wadNumFiles; }
uint32_t WadFormat::getFATOffset() 	{ return wadOffFAT; }
std::string&	WadFormat::getWADName()	{ return wadName; }
std::vector<WadFile>& WadFormat::getWADLumpList() { return wadFiles; }
WadFile& WadFormat::getFileFromIndex(const unsigned int index) { return wadFiles[index]; }
WadFile& WadFormat::operator[](const unsigned int index) { return WadFormat::getFileFromIndex(index); }