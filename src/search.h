#pragma once

#include "utils.h"
#include "moveGen.h"
#include <array>
#include <bit>

constexpr int INF = 1000000000;

namespace Search {
	bool isRepetition(BoardState& boardState, SearchContext& ctx);

	int evaluateM(BoardState& boardState);

	int evaluate(BoardState& boardState);

	int negamax(BoardState& boardState, SearchContext& ctx, int depth, int ply = 1, int alpha = -INF, int beta = +INF);

	Move getBestMove(BoardState& boardState, SearchContext& ctx, int depth);

	int getScore();

	int getNodes();
}