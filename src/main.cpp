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
#include <cstring>
#include <utility>
#include <filesystem>
#include <cmath>

#include "headers/wadformat.h"
#define VERSION_STRING	"v1.0"

enum CompressAction
{
	NoCompress = 0,
	ShouldCompress = 1,
	ShouldDecompress = 2
};

enum PositionAction
{
	NoChange = 0,
	Swap = 1,
	Move = 2
};

std::string_view unknownMessage = "Usage: wadcli [wad file] [arguments]...\n"
	"Try 'wadcli --help' for more information.\n";

int main(int argc, char const *argv[])
{
	if (argc <= 1)
	{
		std::cout << unknownMessage;
		return 0;
	}

	/*
		No argument				// Reads the file.
		--create				// Creates the wad if it does not exist.
		-a, --add  [f1 ...]		// Add file(s) to wad
		--within [marker]		// Adds files inside the markers provided.
								// Partial matches supported: F and F_START will work.
		-d, --delete  [f1 ...] 	// Delete file(s) from wad by file name or by index (using ?num)
		--delete 
		-o,	--overwrite			// To be used alongside -a, overwrites files if they exist. 
		-rn, --rename [f1 ...]	// Rename file(s) from wad, first is file name, second is new name
								// If using --add, then the files being added can be renamed beforehand.
		-m, --merge [f1 ...]	// Merges multiple wads' lumps together.
		-s,	--swap				// Swaps the positions of two lumps provided through --input.
		-p, --position [num]	// Changes the position of a lump provided through --input.
								// Can be absolute (num) or relative (+num or -num).
		-i, --input [f1 ...]	// The input used to --rename or --position or --swap files.
		--create-markers [n1 ..] // Creates  _START and _END markers based on input.
		-c, --compress			// Compresses a IWAD or PWAD into a ZWAD
		-dc, --decompress [P/IWAD] // Decompresses a ZWAD into an IWAD or PWAD (this is an argument)
		--help					// Displays this useful information.
		--version				// Displays a version string.
	*/

	if (strcmp(argv[1], "--help") == 0)
	{
		std::cout <<
		"Usage: wadcli [WAD file] [arguments]...\n"
		"Allows reading, creating, adding or removing files, (de)compress a WAD, etc..\n\n"

		"Read a WAD file by providing a WAD file and no arguments.\n"
		"It is possible to create wads using --create, which will create a WAD\n"
		"if the WAD does not exist.\n\n"

		"Arguments are as follows:\n"
		"--create [P/I/Z]\tCreate the WAD file if it does not exist.\n"
		"-a, --add [f1 ...]\tAdd file(s) to WAD.\n"
		"--within [marker]\tAdd files inside the markers provided.\n"
		"\t\t\tPartial matches supported: F and F_START will work.\n"
		"-d, --delete [f1 ...]\tDelete file(s) from WAD by file name\n"
		"--remove [f1 ...]\n"
		"\t\t\tor by index (using ?num).\n"
		"-o, --overwrite\t\tTo be used alongside -a, overwrites files if they exist.\n"
		"-e, --extract [f1 ...]\tExtracts selected lumps from the WAD.\n"
		"--export [f1 ...]\n"
		"--extract-all\t\tExtracts all lumps from the WAD.\n"
		"--no-extension\t\tNo extension will be added to exported files.\n"
		"--path\t\t\tPath to export the files to.\n"
		"-rn, --rename [f1 ...]\tRename file(s) from WAD\n"
		"\t\t\tIf using --add or --input, then the files being added\n"
		"\t\t\tcan be renamed beforehand, in the order passed to.\n"
		"-s, --swap\t\tSwaps positions of two lumps provided through --input.\n"
		"-p, --position [num]\tChanges the position of a lump provided through --input.\n"
		"\t\t\tCan be absolute (num) or relative (+num or -num).\n"
		"-m, --merge [f1 ...]\tMerges multiple WAD' lumps together.\n"
		"-i, --input [f1 ...]\tThe input used for --rename (without --add),\n"
		"\t\t\t--swap, --posiiton, and --merge.\n"
		"--create-markers [n1..]\tCreates _START and _END markers based on input.\n"
		"\t\t\t_START and _END optional - at least one character\n"
		"\t\t\tis needed to create markers.\n"
		"-c, --compress\t\tCompresses a IWAD or PWAD into a ZWAD\n"
		"-dc, --decompress [P/I]\tDecompresses a ZWAD into an PWAD or IWAD.\n"
		"--output [file]\t\tIf set, a new WAD will be exported\n"
		"\t\t\tusing the set file name.\n"
		"\t\t\tOtherwise, the WAD will be overwritten.\n"
		"--help\t\t\tDisplays this useful information.\n"
		"--version\t\tDisplays a version string and licenses.\n";

		return 0;
	}

	if (strcmp(argv[1], "--version") == 0)
	{
		std::cout <<
		"wadcli " << VERSION_STRING << " by JugadorXEI (https://github.com/JugadorXEI/)\n"
		"Fun fact: WAD stands for Where's All (my) Data.\n\n"

		"Uses LibLZF - Please see LICENSE-3RD-PARTY.txt to see 3rd party licenses.\n";
		return 0;
	}

	// Here's where we determine what to do with the input given.
	// Input - Output
	std::string wadFileName		{};
	std::string_view outputName	{};

	// Compression
	CompressAction compressAction	{ CompressAction::NoCompress };
	WadType wadTypeAfterDecompress	{ INVALID };

	// Deleting files
	bool removingFiles				{ false };
	std::vector<std::string> filesToRemove{};

	// Adding files
	bool addingFiles				{ false };
	std::vector<std::string> filesToAdd{};

	// Renaming files
	bool renamingFiles				{ false };
	std::vector<std::string> filesToRename{};

	// Input files
	bool inputtedFiles				{ false };
	std::vector<std::string> filesToInput{};

	// Overriding files
	bool overridingFiles			{ false };

	// Create the WAD
	bool createWADIfPossible		{ false };
	WadType typeOfWADToCreate		{ INVALID };

	// Create markers
	bool createMarkers				{ false };
	std::vector<std::string> markersToCreate{};

	// Merging WADS
	bool mergingWADs				{ false };
	std::vector<std::string> wadsToMerge{};

	// Extract lumps
	bool extractLumps				{ false };
	std::vector<std::string> lumpsToExtract{};

	// Extract all lumps
	bool extractAllLumps			{ false };

	// No extension on export
	bool noExtensionOnExport		{ false };

	// Path to export to.
	std::string exportPath{};

	// Swap or change lump positions
	PositionAction changePositions { PositionAction::NoChange };
	bool changePositionsRelative { false };
	int toWhichPosition	{ 0 };

	// Add within markers
	bool addFilesWithinMarkers		{ false };
	std::string markerName{};

	// Process parameters.
	for (size_t i = 1; i < static_cast<size_t>(argc); ++i)
	{
		if constexpr (DEBUG) std::cout << i << ": " << argv[i] << '\n';
		if (i == 1) // Ignore argument in the first input.
		{
			if (argv[i][0] == '-') // Is the first parameter an argument?
			{
				// It should be a wad instead.
				std::cout << "WADCLI: Error: Pass a WAD file first before using any arguments!\n" <<
					unknownMessage;
				return 0;
			}
			else
			{
				wadFileName = argv[i];
			}

			continue;
		}

		bool* booleanToChange{ nullptr };
		std::vector<std::string>* listToAddTo{ nullptr };
		std::string operationType{ "error" };

		if (compressAction == CompressAction::NoCompress &&
			(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compress") == 0))
		{
			compressAction = ShouldCompress;
			continue;
		}
		else if (compressAction == CompressAction::NoCompress &&
			(strcmp(argv[i], "-dc") == 0 || strcmp(argv[i], "--decompress") == 0))
		{
			compressAction = ShouldDecompress;
		
			char character = '0';
			if (i < static_cast<size_t>(argc - 1))
				character = argv[++i][0];

			switch (character)
			{
				case 'I': wadTypeAfterDecompress = IWAD; break;
				default:  wadTypeAfterDecompress = PWAD; break;
			}

			continue;
		}
		else if (changePositions == PositionAction::NoChange &&
			(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--swap") == 0))
		{
			changePositions = PositionAction::Swap;
			continue;
		}
		else if (changePositions == PositionAction::NoChange &&
			(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--position") == 0))
		{
			changePositions = PositionAction::Move;

			std::string_view positionString{};
			if (i < static_cast<size_t>(argc - 1))
				positionString = argv[++i];

			if (positionString.empty())
			{
				std::cout << "WADCLI: Used --position without setting a position!\n"; 
				return 0;
			}

			if (positionString[0] == '+' || positionString[0] == '-')
				changePositionsRelative = true;		

			toWhichPosition = atoi(positionString.data());
			if (toWhichPosition == 0)
			{
				std::cout << "WADCLI: --position's number argument parsed as zero, may be invalid.\n";
				return 0;
			}

			if constexpr (DEBUG)
				std::cout << toWhichPosition << " - relative: " << changePositionsRelative << '\n';

			continue;
		}
		else if (!addFilesWithinMarkers && strcmp(argv[i], "--within") == 0)
		{
			addFilesWithinMarkers = true;

			if (i < static_cast<size_t>(argc - 1))
				markerName = argv[++i];

			if (markerName.empty())
			{
				std::cout << "WADCLI: Used --within without setting a marker!\n";
				return 0;
			}

			if (markerName.size() > 2)
				WadFormat::trimStringToMarkerCharacters(markerName);

			markerName += "_END";

			continue;
		}
		else if (!overridingFiles && (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--override") == 0))
		{
			overridingFiles = true;
			continue;
		}
		else if (strcmp(argv[i], "--create") == 0)
		{
			createWADIfPossible = true;

			char character = '0';
			if (i < static_cast<size_t>(argc - 1))
				character = argv[++i][0];

			switch (character)
			{
				case 'I': typeOfWADToCreate = IWAD; break;
				case 'Z': typeOfWADToCreate = ZWAD; break;
				default:  typeOfWADToCreate = PWAD; break;
			}

			continue;
		}
		else if (strcmp(argv[i], "--output") == 0)
		{
			if (i < static_cast<size_t>(argc - 1))
				outputName = argv[++i];

			if (outputName.empty())
			{
				std::cout << "WADCLI: Used --output without setting any file name!\n"; 
				return 0;
			}

			continue;
		}
		else if (strcmp(argv[i], "--extract-all") == 0)
		{
			extractAllLumps = true;
			continue;	
		}
		else if (strcmp(argv[i], "--no-extension") == 0)
		{
			noExtensionOnExport = true;
			continue;
		}
		else if (strcmp(argv[i], "--path") == 0)
		{
			if (i < static_cast<size_t>(argc - 1))
				exportPath = argv[++i];

			if (exportPath.empty() || exportPath[0] == '-')
			{
				std::cout << "WADCLI: Used --path without setting any path to export!\n"; 
				return 0;
			}

			std::filesystem::path pathToExport{ exportPath }; 
			if (!std::filesystem::exists(pathToExport))
			{
				std::error_code error;
				std::filesystem::create_directories(pathToExport, error);

				if (error)
				{
					std::cout << "WADCLI: An error has been found creating folders.\n" <<
						error.message() << '\n';
					return 0;
				}

				std::cout << "WADCLI: Successfully created folders: " << pathToExport << '\n';

				/*
				std::cout << "WADCLI: Please point to a directory to export to!\n";
				std::cout << pathToExport << '\n';
				return 0;
				*/
			}

			if (exportPath[exportPath.size()] != std::filesystem::path::preferred_separator)
				exportPath += std::filesystem::path::preferred_separator;

			continue;
		}
		// These are operations that take a list of arguments.
		else if (strcmp(argv[i], "-d") == 0 ||
			strcmp(argv[i], "--remove") == 0 ||
			strcmp(argv[i], "--delete") == 0)
		{
			booleanToChange = &removingFiles;
			listToAddTo 	= &filesToRemove;
			operationType	= "delete";
		}
		else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--add") == 0)
		{
			booleanToChange = &addingFiles;
			listToAddTo 	= &filesToAdd;
			operationType 	= "add";
		}
		else if (strcmp(argv[i], "-rn") == 0 || strcmp(argv[i], "--rename") == 0)
		{
			booleanToChange = &renamingFiles;
			listToAddTo 	= &filesToRename;
			operationType	= "rename";
		}
		else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0)
		{
			booleanToChange = &inputtedFiles;
			listToAddTo 	= &filesToInput;
			operationType	= "input";
		}
		else if (strcmp(argv[i], "--create-markers") == 0)
		{
			booleanToChange = &createMarkers;
			listToAddTo		= &markersToCreate;
			operationType	= "marker creation";
		}
		else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--merge") == 0)
		{
			booleanToChange = &mergingWADs;
			listToAddTo		= &wadsToMerge;
			operationType	= "merging";
		}
		else if (strcmp(argv[i], "-e") == 0 ||
			strcmp(argv[i], "--extract") == 0 ||
			strcmp(argv[i], "--export") == 0)
		{
			booleanToChange = &extractLumps;
			listToAddTo		= &lumpsToExtract;
			operationType	= "extract";			
		}
		else
		{
			std::cout << "WADCLI: Unknown argument: " << argv[i] << '\n';
			return 0;
		}

		size_t howMany{ 0 };

		++i;
		while (i < static_cast<size_t>(argc) && argv[i][0] != '-') // Don't add arguments, that'd be silly.
		{
			++howMany;

			(*listToAddTo).push_back(argv[i]);
			if constexpr (DEBUG)
				std::cout << i << ": " << argv[i] << " (" << operationType << ")\n";

			++i;
		}
		--i; // Since we're at a parameter, continuing the loop would skip it.

		if (howMany > 0)
			(*booleanToChange) = true;
		else
		{
			// Nothing? Let's assume it's a mistake and stop.
			std::cout << "WADCLI: Used an " << operationType.c_str() << " operation without any arguments!\n"; 
			return 0;
		}

		// extra validation in case we're the last.
		if (i == static_cast<size_t>(argc))
			break;
			
		continue;
	}

	if (wadFileName.empty())
	{
		std::cout << "Error: no file name was given for WAD!\n";
		std::cout << unknownMessage;
		return 0;
	}

	// Let's create the wad object.
	WadFormat wad{ wadFileName, typeOfWADToCreate };
	if (std::filesystem::exists(wadFileName))
	{
		if (!wad.importWAD(wadFileName))
		{
			std::cout << "WADCLI: There was an error reading " <<
				std::quoted(wadFileName) << ".\n" <<
				"We are not allowed to read it.\n";
			return 0;
		}
	}
	else if (!createWADIfPossible)
	{
		std::cout << "WADCLI: There was an error reading " << 
			std::quoted(wadFileName) << ".\n" <<
			"It may not exist.\n";
		return 0;
	}

	if (!exportPath.empty() && !(extractLumps || extractAllLumps || !outputName.empty()))
	{
		std::cout << "WADCLI: --path requires --extract, --extract-all or --output.\n" <<
			"Did you mean to use --input instead?\n";
		return 0;
	}

	if (changePositions != PositionAction::NoChange && !inputtedFiles)
	{
		std::cout << "WADCLI: --position/--swap require --input to change/swap lump locations!\n";
		return 0;
	}

	if (changePositions == PositionAction::Swap && filesToInput.size() > 2)
	{
		std::cout << "WADCLI: With --swap, only two lumps can be swapped at a time!\n";
		return 0;
	}

	// We're just reading the file.
	if (!wadFileName.empty() and argc == 2)
	{
		std::cout << "WAD: " << wad.getWADName() << " (" << wad.getWADTypeToChar() << ")" << '\n';

		unsigned int numFiles = wad.getNumFiles();
		std::cout << "Files (" << numFiles << "):\n";
		for (unsigned int i = 0; i < numFiles; ++i)
		{
			std::cout << "File " << (i + 1) << ": " << wad[i].name << " (Size: " << wad[i].dataSize << ", Offset: " << wad[i].dataOffset << ")" << '\n';
		}

		std::cout << "Done reading " << wad.getWADName() << '\n';
		return 0;
	}

	// Merging WADs
	if (mergingWADs)
	{
		for (std::string& name : wadsToMerge)
		{
			if (!std::filesystem::exists(name))
			{
				std::cout << "WADCLI: There was an error reading " << name << '\n' <<
					"It may not exist.\n";
				continue;
			}

			WadFormat mergingWAD{};
			if (!mergingWAD.importWAD(name))
			{
				std::cout << "WADCLI: There was an error reading " << name << '\n' <<
					"We do not have permission to read it.\n";
				continue;
			}

			if (wad.getWADType() != mergingWAD.getWADType())
			{
				if (wad.getWADType() == ZWAD && mergingWAD.getWADType() != ZWAD)
					mergingWAD.compressWAD();
				else if (wad.getWADType() != ZWAD && mergingWAD.getWADType() == ZWAD)
					mergingWAD.decompressWAD();
			}

			// We're goooooood.
			for (WadFile& lump : mergingWAD.getWADLumpList())
				wad.addFileToWAD(lump);

			std::cout << "WADCLI: Done merging WAD " << name <<
				" into " << wadFileName << ".\n";
		}
	}

	// Yay, removing files!
	if (removingFiles)
	{
		for (std::string& name : filesToRemove)
		{
			if (int index = atoi(name.substr(1, name.size()).c_str()) - 1; name[0] == '?')
			{
				if constexpr (DEBUG) 
					std::cout << "removing " << index << " \n";
				// by index.
				if (index > -1 && static_cast<unsigned int>(index) < wad.getNumFiles())
				{
					wad.removeFileByIndex(static_cast<unsigned int>(index));
					std::cout << "WADCLI: Removed file #" << index << " from WAD.\n";
				}
			}
			else if (wad.removeFileByName(name))
				std::cout << "WADCLI: Removed file " << name << " from WAD.\n";
			else
				std::cout << "WADCLI: Could not find file " << name << " to remove from WAD.\n";
		}	
	}

	if (createMarkers)
	{
		// Sanity-check the marker names.
		for (std::string& markerName : markersToCreate)
		{
			if (markerName.size() > 2)
				WadFormat::trimStringToMarkerCharacters(markerName);

			wad.createMarkers(markerName);
		}
	}

	// Yay, adding files!
	if (addingFiles)
	{
		size_t i{ 0 };
		size_t success{ 0 };
		for (std::string& name : filesToAdd)
		{
			int markerIndex{ -1 };
			if (addFilesWithinMarkers)
			{
				for (size_t i = 0; i < wad.getNumFiles(); ++i)
				{
					if (strcmp(wad[i].name.c_str(), markerName.c_str()) == 0)
					{
						markerIndex = i;
						break;
					}
				}

				if (markerIndex == -1)
				{
					std::cout << "WADCLI: Could not find marker " << markerName << ", " <<
						"lumps will be placed at the end of the WAD.\n";
					addFilesWithinMarkers = false;
				}
			}


			if (wad.addFileToWAD(name.c_str(),
				(!filesToRename.empty() && filesToRename[i].size() > 0 ? filesToRename[i] : ""),
				overridingFiles))
			{
				std::string realName{ !filesToRename.empty() && filesToRename[i].size() > 0 ? filesToRename[i] : name.c_str() };
				std::cout << "WADCLI: Added file " << realName << " to WAD.\n";

				if (addFilesWithinMarkers)
				{
					if (wad.moveLumpPosByName(realName.c_str(), markerIndex, false))
						std::cout << "WADCLI: Moved " << realName << " to position " <<
							markerIndex << ".\n"; 
				}	

				++success;
			}
			else
				std::cout << "WADCLI: Can't read file " << name << ": it may not exist " <<
					"or we are not allowed to read it.\n";

			++i;
		}

		if (success == 0)
		{
			std::cout << "WADCLI: Error: there was trouble adding all files. Quitting early.\n";
			return 0;
		}
	}

	if (!addingFiles && renamingFiles)
	{
		if (!inputtedFiles)
		{
			std::cout << "WADCLI: You must --input the files you wish to rename!\n";
			return 0;
		}

		size_t i{ 0 };
		for (std::string& lumpName : filesToInput)
		{
			// handle cases where there's less renames than inputs.
			if (i == filesToRename.size())
				break;

			bool couldFindIt{ false };

			for (WadFile& lump : wad.getWADLumpList())
			{
				if (strcmp(lumpName.c_str(), lump.name.c_str()) == 0)
				{
					// We found it, so now we're renaming it.
					lump.name = filesToRename[i];
					couldFindIt = true;
					break;
				}
			}

			if (couldFindIt)
				std::cout << "WADCLI: Successfully renamed " << lumpName <<
					" into " << filesToRename[i].c_str() << ".\n";
			else
				std::cout << "WADCLI: Could not find lump " << lumpName <<
					" to rename into " << filesToRename[i].c_str() << ".\n";

			i++;
		}
			
		// No fail condition in case rename fails because it would be a little weird...
		// Just tell the user that they couldn't find the lump.
	}

	if (changePositions)
	{
		if (changePositions == PositionAction::Swap)
		{
			if (wad.swapLumpPosByName(filesToInput[0], filesToInput[1]))
			{
				std::cout << "WADCLI: Lumps " << filesToInput[0] <<
					" and " << filesToInput[1] << " were successfully swapped.\n";
			}
			else
			{
				std::cout << "WADCLI: Could not find lumps " << filesToInput[0] <<
					" and " << filesToInput[1] << " to swap.\n";
				// set to no change - if this was the only thing done, then the wad will not re-export.
				changePositions = PositionAction::NoChange;
			}
		}
		else if (changePositions == PositionAction::Move)
		{
			for (std::string& lumpName : filesToInput)
			{
				if (wad.moveLumpPosByName(lumpName,
					changePositionsRelative ? toWhichPosition : toWhichPosition - 1,
					changePositionsRelative))
				{
					std::cout << "WADCLI: " << lumpName << " moved successfully to " <<
						(changePositionsRelative ?
						(std::signbit(toWhichPosition) ? "-" : "+")
						 : "") << std::abs(toWhichPosition) <<
						(changePositionsRelative ? " (relative)" : "") << ".\n";
				}
				else
					std::cout << "WADCLI: We either could not find the lump " << lumpName << '\n' <<
						"to move, or the movement would have made the lump\n" << 
						"go out of bounds.\n";
			}
		}
	}

	if (extractAllLumps)
	{
		for (WadFile& lump : wad.getWADLumpList())
		{
			// We found it, so now we're extracting it.
			if (wad.extractLump(lump, noExtensionOnExport, exportPath.empty() ? "" : exportPath))
				std::cout << "WADCLI: Successfully extracted " << lump.name << ".\n";
		}
	}
	else if (extractLumps)
	{
		if (lumpsToExtract.empty())
		{
			std::cout << "WADCLI: You must provide the lump names you wish to extract!\n";
			return 0;
		}

		for (std::string& lumpName : lumpsToExtract)
		{
			bool wasFound{ false };

			for (WadFile& lump : wad.getWADLumpList())
			{
				if (strcmp(lumpName.c_str(), lump.name.c_str()) == 0)
				{
					// We found it, so now we're extracting it.
					if (wad.extractLump(lump, noExtensionOnExport, exportPath.empty() ? "" : exportPath))
					{
						std::cout << "WADCLI: Successfully extracted " << lumpName << ".\n";
						wasFound = true;
					}

					break;
				}
			}

			if (!wasFound)
				std::cout << "WADCLI: Could not find lump " << lumpName << ".\n";
		}
	}

	// Yay, compression!
	if (compressAction != NoCompress)
	{
		if (compressAction == ShouldCompress)
		{
			if (wad.getWADType() == ZWAD)
			{
				std::cout << "WADCLI: Error: can't compress an already-compressed ZWAD!\n";
				return 0;
			}

			std::cout << "WADCLI: Compressing WAD " << wad.getWADName() << "...\n";

			if (wad.compressWAD())
				std::cout << "WADCLI: Compressed WAD successfully.\n";
			else
			{
				std::cout << "WADCLI: There was an error compressing the WAD!\n";
				return 0;
			}
				
		}
		else if (compressAction == ShouldDecompress)
		{
			if (wad.getWADType() != ZWAD)
			{
				std::cout << "WADCLI: Error: can only decompress ZWADs!\n";
				return 0;
			}

			std::cout << "WADCLI: Decompressing ZWAD " << wad.getWADName() << "...\n";
			
			if (wad.decompressWAD(wadTypeAfterDecompress))
				std::cout << "WADCLI: Decompressed WAD successfully.\n";
			else
			{
				std::cout << "WADCLI: There was an error decompressing the WAD!\n";
				return 0;
			}
				
		}
	}

	// We're exporting the new wad.
	if (!outputName.empty())
	{
		if (!exportPath.empty())
			wadFileName = exportPath + outputName.data();
		else
			wadFileName = outputName;

		/*
		std::cout << "Final name:\n" <<
			exportPath << '\n' <<
			outputName << '\n' <<
			wadFileName << '\n';
		*/
	}
		

	// When do we export?
	bool wereAnyChangesDone
	{
		compressAction != NoCompress ||
		addingFiles ||
		removingFiles ||
		renamingFiles ||
		createWADIfPossible ||
		createMarkers ||
		mergingWADs ||
		!outputName.empty() ||
		changePositions != NoChange
	};
	
	if (extractLumps || extractAllLumps)
	{
		std::cout << "WADCLI: All extraction operations finished.\n";
	}

	if (wereAnyChangesDone)
		wad.exportWAD(wadFileName);
	else if (!(extractLumps || extractAllLumps))
	{
		std::cout << "WADCLI: No action was done.\n" <<
			"Arguments might have been misused.\n";
	}

	// std::cout << "Done.\n";
	return 0;
}
