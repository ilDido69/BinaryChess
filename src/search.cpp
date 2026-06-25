#include "search.h"

constexpr int MAXPLY = 128;
const std::array<int, 6> pieceValues = { 100, 320, 330, 500, 900, 20000 };
MoveList moveStack[MAXPLY] = {};
StateInfo savedStack[MAXPLY] = {};

int Search::evaluate(BoardState& boardState)
{
	int score = 0;

	
	for (int piece = PAWN; piece <= KING; piece++)
	{
		score += std::popcount(boardState.bb[WHITE][piece]) * pieceValues[piece];
		score -= std::popcount(boardState.bb[BLACK][piece]) * pieceValues[piece];
	}

	return boardState.sideToMove == WHITE ? score : -score;
}

int Search::negamax(BoardState& boardState, int depth, int ply)
{
	int best = -100000001;

	moveStack[ply].count = 0;
	MoveGen::getLegalMoves(boardState, moveStack[ply]);

	if (moveStack[ply].count == 0)
	{
		if (MoveGen::onCheck(boardState))
			return -100000000 - depth;
		return 0;
	}

	if (depth == 0)
		return evaluate(boardState);

	int result;
	for (int i = 0; i < moveStack[ply].count; i++)
	{
		MoveGen::makeMove(boardState, moveStack[ply].moves[i], savedStack[ply]);
		result = -negamax(boardState, depth - 1, ply + 1);
		MoveGen::unmakeMove(boardState, moveStack[ply].moves[i], savedStack[ply]);
		if (result > best)
			best = result;
	}

	return best;
}

Move Search::getBestMove(BoardState& boardState, int depth)
{
	moveStack[0].count = 0;
	MoveGen::getLegalMoves(boardState, moveStack[0]);

	Move best = encodeMove(0, 0, EMPTY);
	int bestScore = -100000001;

	int result;
	for (int i = 0; i < moveStack[0].count; i++)
	{
		MoveGen::makeMove(boardState, moveStack[0].moves[i], savedStack[0]);
		result = -negamax(boardState, depth - 1);
		MoveGen::unmakeMove(boardState, moveStack[0].moves[i], savedStack[0]);
		if (result > bestScore)
		{
			bestScore = result;
			best = moveStack[0].moves[i];
		}
	}

	return best;
}
