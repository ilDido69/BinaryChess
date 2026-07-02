#pragma once
#include <array>
#include <vector>
#include "utils.h"

class TranspositionTable
{
private:
	std::vector<TTEntry> table;
	size_t nEntries;
	uint64_t mask;
public:
	TranspositionTable(size_t sizeInMB);

	void allocate(size_t sizeInMB);
	void clear();

	bool probe(uint64_t key, int depth, int alpha, int beta, int& ttScore, Move& ttMove);
	void store(uint64_t key, int depth, int score, TTEntry::Flag flag, Move move);
};