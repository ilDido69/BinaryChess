#include "search.h"

const std::array<int, 6> pieceValues = { 100, 320, 330, 500, 900, 20000 };

constexpr std::array<int, 64> pawnValues = {
	 0,  0,  0,  0,  0,  0,  0,  0,
	50, 50, 50, 50, 50, 50, 50, 50,
	10, 10, 20, 30, 30, 20, 10, 10,
	 5,  5, 10, 25, 25, 10,  5,  5,
	 0,  0,  0, 20, 20,  0,  0,  0,
	 5, -5,-10,  0,  0,-10, -5,  5,
	 5, 10, 10,-20,-20, 10, 10,  5,
	 0,  0,  0,  0,  0,  0,  0,  0
};

constexpr std::array<int, 64> knightValues = {
	-50,-40,-30,-30,-30,-30,-40,-50,
	-40,-20,  0,  0,  0,  0,-20,-40,
	-30,  0, 10, 15, 15, 10,  0,-30,
	-30,  5, 15, 20, 20, 15,  5,-30,
	-30,  0, 15, 20, 20, 15,  0,-30,
	-30,  5, 10, 15, 15, 10,  5,-30,
	-40,-20,  0,  5,  5,  0,-20,-40,
	-50,-40,-30,-30,-30,-30,-40,-50,
};

constexpr std::array<int, 64> bishopValues = {
	-20,-10,-10,-10,-10,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5, 10, 10,  5,  0,-10,
	-10,  5,  5, 10, 10,  5,  5,-10,
	-10,  0, 10, 10, 10, 10,  0,-10,
	-10, 10, 10, 10, 10, 10, 10,-10,
	-10,  5,  0,  0,  0,  0,  5,-10,
	-20,-10,-10,-10,-10,-10,-10,-20,
};

constexpr std::array<int, 64> rookValue = {
	  0,  0,  0,  0,  0,  0,  0,  0,
	  5, 10, 10, 10, 10, 10, 10,  5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	  0,  0,  0,  5,  5,  0,  0,  0
};

constexpr std::array<int, 64> queenValues = {
	-20,-10,-10, -5, -5,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5,  5,  5,  5,  0,-10,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	-10,  5,  5,  5,  5,  5,  0,-10,
	-10,  0,  5,  0,  0,  0,  0,-10,
	-20,-10,-10, -5, -5,-10,-10,-20
};

constexpr std::array<int, 64> kingValue = {
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-20,-30,-30,-40,-40,-30,-30,-20,
	-10,-20,-20,-20,-20,-20,-20,-10,
	 20, 20,  0,  0,  0,  0, 20, 20,
	 20, 30, 10,  0,  0, 10, 30, 20
};

const std::array<int, 64>* psts[6] = {
		&pawnValues, &knightValues, &bishopValues,
		&rookValue, &queenValues, &kingValue
};

bool Search::isRepetition(BoardState& boardState, SearchContext& ctx)
{
	if (boardState.rule50 < 4) return false;

	int limit = ctx.historyCount - boardState.rule50;
	if (limit < 0) limit = 0;

	for (int i = ctx.historyCount - 2; i >= limit; i -= 2)
	{
		if (ctx.hashHistory[i] == boardState.hash)
			return true;
	}
	return false;
}

int Search::evaluateM(BoardState& boardState)
{
	int score = 0;

	
	for (int piece = PAWN; piece <= KING; piece++)
	{
		score += std::popcount(boardState.bb[WHITE][piece]) * pieceValues[piece];
		score -= std::popcount(boardState.bb[BLACK][piece]) * pieceValues[piece];
	}

	return boardState.sideToMove == WHITE ? score : -score;
}


int Search::evaluate(BoardState& boardState)
{
	int score = 0;

	for (int piece = PAWN; piece <= KING; piece++)
	{
		uint64_t bbWhite = boardState.bb[WHITE][piece];
		while (bbWhite)
		{
			int sq = std::countr_zero(bbWhite);
			bbWhite &= bbWhite - 1;

			score += pieceValues[piece];
			score += (*psts[piece])[sq];
		}

		uint64_t bbBlack = boardState.bb[BLACK][piece];
		while (bbBlack)
		{
			int sq = std::countr_zero(bbBlack);
			bbBlack &= bbBlack - 1;

			score -= pieceValues[piece];

			score -= (*psts[piece])[sq ^ 56];
		}
	}
	return boardState.sideToMove == WHITE ? score : -score;
}

int Search::negamax(BoardState& boardState, SearchContext& ctx, int depth, int ply, int alpha, int beta)
{
	int best = -INF;
	if (boardState.rule50 >= 100 || isRepetition(boardState, ctx))
		return 0;

	ctx.moveStack[ply].count = 0;
	MoveGen::getLegalMoves(boardState, ctx.moveStack[ply]);

	if (ctx.moveStack[ply].count == 0)
	{
		if (MoveGen::onCheck(boardState))
			return -100000000 - depth;
		return 0;
	}

	if (depth == 0)
		return evaluate(boardState);

	int result;
	for (int i = 0; i < ctx.moveStack[ply].count; i++)
	{
		MoveGen::makeMove(boardState, ctx.moveStack[ply].moves[i], ctx.stateStack[ply]);
		ctx.pushHash(boardState.hash);
		result = -negamax(boardState, ctx, depth - 1, ply + 1, -beta, -alpha);
		ctx.popHash();
		MoveGen::unmakeMove(boardState, ctx.moveStack[ply].moves[i], ctx.stateStack[ply]);
		if (result > best)
		{
			best = result;
		}
		if (result > alpha)
		{
			alpha = result;
		}
		if (alpha >= beta)
			break;
	}

	return best;
}

int bestScore;

Move Search::getBestMove(BoardState& boardState, SearchContext& ctx, int depth)
{
	ctx.moveStack[0].count = 0;
	MoveGen::getLegalMoves(boardState, ctx.moveStack[0]);

	Move best = encodeMove(0, 0, EMPTY);
	bestScore = -INF;

	int result;
	for (int i = 0; i < ctx.moveStack[0].count; i++)
	{
		MoveGen::makeMove(boardState, ctx.moveStack[0].moves[i], ctx.stateStack[0]);
		ctx.pushHash(boardState.hash);
		result = -negamax(boardState, ctx, depth - 1);
		ctx.popHash();
		MoveGen::unmakeMove(boardState, ctx.moveStack[0].moves[i], ctx.stateStack[0]);
		if (result > bestScore)
		{
			bestScore = result;
			best = ctx.moveStack[0].moves[i];
		}
	}

	return best;
}

int Search::getScore()
{
	return bestScore;
}

int Search::getNodes()
{
	return 0;
}


