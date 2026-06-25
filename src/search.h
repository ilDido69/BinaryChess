#pragma once

#include "utils.h"
#include "moveGen.h"
#include <array>
#include <bit>

constexpr int INF = 1000000000;

namespace Search {
	int evaluateM(BoardState& boardState);

	int evaluate(BoardState& boardState);

	int negamax(BoardState& boardState, int depth, int ply = 1, int alpha = -INF, int beta = +INF);

	Move getBestMove(BoardState& boardState, int depth);
}