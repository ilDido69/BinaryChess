#pragma once

#include "utils.h"
#include "moveGen.h"
#include <array>
#include <bit>

namespace Search {
	int evaluate(BoardState& boardState);

	int negamax(BoardState& boardState, int depth, int ply = 1);

	Move getBestMove(BoardState& boardState, int depth);
}