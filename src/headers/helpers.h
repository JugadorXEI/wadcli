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

#ifndef JUG_HELPERS_H
#define JUG_HELPERS_H

enum wildcard
{
	Any = '?',
	All = '*'
};

bool wildcardMatch(const char* string, const char* pattern,
	unsigned int stringSize, unsigned int patternSize)
{
	bool skipUntilNextMatch{ false };

	for (unsigned int i = 0, j = 0; (i < patternSize && j < stringSize);)
	{
		if constexpr (DEBUG)
			std::cout << pattern[i] << " = " << string[j] << " (skipping: " << skipUntilNextMatch << ")\n";

		if (pattern[i] == wildcard::All)
		{
			if (skipUntilNextMatch) return false; // We couldn't find the pattern.
			else skipUntilNextMatch = true;
			
			++i; // Advance the pattern, not the string.
			if (i == patternSize) // If there's nothing else, let's just return true.
				return true;

			continue;
		}

		if (pattern[i] == wildcard::Any)
		{
			// Advance both the pattern and string.
			++i;
			++j;
			continue;
		}
			

		if (pattern[i] == string[j])
		{
			if (skipUntilNextMatch)
				skipUntilNextMatch = false;

			// Advance both the pattern and string.
			++i;
			++j;
			continue;
		}
		else if (skipUntilNextMatch)
		{
			// We're skipping, so we're
			// advancing the string.
			++j;
			continue;
		}
		else return false;
	}

	return false;
}

#endif