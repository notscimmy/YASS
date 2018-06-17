#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <assert.h>

namespace PatternScan
{
	class Pattern
	{
	public:
		std::string opcodes;
		std::string mask;

		Pattern(std::string opcodes, std::string mask) : opcodes(opcodes), mask(mask) {}

		std::string ToString()
		{
			std::string result;
			result += opcodes + ", " + mask;
			return result;
		}
	};

	class YASS
	{
	public:
		bool CompareBytes(const unsigned char* address, const unsigned char* opcodes, const char* mask)
		{
			while (*mask)
			{
				if (*mask == 'x' && *address != *opcodes)
					return false;

				++mask;
				++address;
				++opcodes;
			}
			return true;
		}

		uintptr_t FindPattern(uintptr_t startAddress, size_t length, const unsigned char* opcodes, const char* mask, int ignore)
		{
			int ignoreCount = 0;
			uintptr_t offset = 0;

			while (ignoreCount <= ignore)
			{
				if (CompareBytes((unsigned char*)(startAddress + offset++), opcodes, mask))
					++ignoreCount;
				else if (offset >= length)
					return 0;
			}

			return startAddress + offset - 1;
		}

		void PermuteVectorHelper(std::vector<std::string> &orig, std::vector<std::string> &perm, std::vector<std::string> &output)
		{
			if (orig.empty())
			{
				std::string permutation;
				for (int i = 0; i < perm.size(); i++)
					permutation += perm[i];

				output.push_back(permutation);
				return;
			}

			for (int i = 0; i < orig.size(); ++i)
			{
				std::vector<std::string> origCopy(orig);
				origCopy.erase(std::find(origCopy.begin(), origCopy.end(), origCopy.at(i)));
				std::vector<std::string> permCopy(perm);
				permCopy.push_back(orig.at(i));
				PermuteVectorHelper(origCopy, permCopy, output);
			}
		}

		void PermuteVector(std::vector<std::string> &orig, std::vector<std::string> &output)
		{
			std::vector<std::string> perm;
			PermuteVectorHelper(orig, perm, output);
		}

		std::vector<Pattern> GeneratePermutations()
		{
			PermuteVector(instructions, instructionPermutations);
			PermuteVector(masks, maskPermutations);

			std::vector<Pattern> permutations;
			for (int i = 0; i < instructionPermutations.size(); i++)
				permutations.push_back({ instructionPermutations[i], maskPermutations[i] });

			return permutations;
		}

		std::vector<std::string> instructions;
		std::vector<std::string> masks;
		std::vector<std::string> instructionPermutations;
		std::vector<std::string> maskPermutations;

	public:
		YASS(std::vector<std::string> instructions, std::vector<std::string> masks) : instructions(instructions), masks(masks)
		{
			assert(instructions.size() > 0 && masks.size() > 0);
			assert(instructions.size() == masks.size());
		}

		uintptr_t Scan(uintptr_t startAddress, size_t length)
		{
			std::vector<Pattern> patterns = GeneratePermutations();

			for (Pattern pattern : patterns)
			{
				uintptr_t result = FindPattern(startAddress, length, (unsigned char*)pattern.opcodes.c_str(), pattern.mask.c_str(), 0);
				if (result)
					return result;
			}

			return 0;
		}
	};
}