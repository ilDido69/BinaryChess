#include "search.h"
#include "tt.h"

TranspositionTable tt(2048);


const std::array<int, 6> pieceValues = { 100, 320, 330, 500, 900, 20000 };
const int promoScores[6] = { 0, 30'000, 30'000, 40'000, 800'000, 0 };
const int promoCapScores[6] = { 0, 30'000, 30'000, 40'000, 900'000, 0 };
const int flagBaseScores[8] = {
	0,          // QUIET
	0,          // CAPTURE
	1900,       // EN_PASSANT
	50,         // CASTLE_K
	50,         // CASTLE_Q
	0,          // PROMO
	0           // PROMO_CAP
};

// Rows = Victims (P, N, B, R, Q), Columns = Attackers (P, N, B, R, Q, K)
inline constexpr int mvvLvaTable[5][6] = {
	{1900,  1700,  1700,  1500,  1100,  1950},
	{3900,  3700,  3700,  3500,  3100,  3950},
	{3900,  3700,  3700,  3500,  3100,  3950},
	{5900,  5700,  5700,  5500,  5100,  5950},
	{9900,  9700,  9700,  9500,  9100,  9950}
};

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

int Search::scoreMove(Move move, BoardState& boardState, Move TTmove)
{
	if (move == TTmove) return 10'000'000;

	MoveFlag flag = getFlag(move);
	int score = flagBaseScores[flag];

	if (flag == CAPTURE || flag == PROMO_CAP) {
		int to = getTo(move);
		Piece captured = getPiece(boardState, ~boardState.sideToMove, to);
		Piece moved = getMoved(move);
		score += mvvLvaTable[captured][moved];
	}

	if (flag == PROMO || flag == PROMO_CAP) {
		Piece promo = getPromo(move);
		score += (flag == PROMO) ? promoScores[promo] : promoCapScores[promo];
	}

	return score;
}

/*
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
*/
long long nodesCount;

int Search::negamax(BoardState& boardState, SearchContext& ctx, int depth, int ply, int alpha, int beta) 
{
	nodesCount++;

	if (ply > 1 && (boardState.rule50 >= 100 || isRepetition(boardState, ctx)))
		return 0;

	if (depth == 0) {
		return evaluate(boardState);
	}

	int ttScore = 0;
	Move ttMove;

	if (tt.probe(boardState.hash, depth, alpha, beta, ttScore, ttMove)) {

		if (ttScore > MATE)  ttScore -= ply;
		if (ttScore < -MATE) ttScore += ply;

		return ttScore;
	}

	ctx.moveStack[ply].count = 0;
	MoveGen::getLegalMoves(boardState, ctx.moveStack[ply]);
	if (ctx.moveStack[ply].count == 0)
	{
		if (MoveGen::onCheck(boardState))
			return -100000000 + ply;
		return 0;
	}

	
	for (int i = 0; i < ctx.moveStack[ply].count; i++)
	{
		ctx.moveScores[ply][i] = scoreMove(ctx.moveStack[ply].moves[i], boardState, ttMove);
	}


	int best = -INF;
	Move bestMove = 0;
	int originalAlpha = alpha;
	int result;
	for (int i = 0; i < ctx.moveStack[ply].count; i++)
	{

		int bestMoveIndex = i;
		for (int j = i + 1; j < ctx.moveStack[ply].count; j++)
		{
			if (ctx.moveScores[ply][j] > ctx.moveScores[ply][bestMoveIndex])
			{
				bestMoveIndex = j;
			}
		}

		std::swap(ctx.moveStack[ply].moves[i], ctx.moveStack[ply].moves[bestMoveIndex]);
		std::swap(ctx.moveScores[ply][i], ctx.moveScores[ply][bestMoveIndex]);

		MoveGen::makeMove(boardState, ctx.moveStack[ply].moves[i], ctx.stateStack[ply]);
		ctx.pushHash(boardState.hash);
		result = -negamax(boardState, ctx, depth - 1, ply + 1, -beta, -alpha);
		ctx.popHash();
		MoveGen::unmakeMove(boardState, ctx.moveStack[ply].moves[i], ctx.stateStack[ply]);
		if (result > best)
		{
			best = result;
			bestMove = ctx.moveStack[ply].moves[i];
		}
		if (result > alpha)
		{
			alpha = result;
		}
		if (alpha >= beta)
			break;
	}
	
	TTEntry::Flag flag;
	if (best <= originalAlpha) {
		flag = TTEntry::UPPERBOUND;
	}
	else if (best >= beta) {
		flag = TTEntry::LOWERBOUND;
	}
	else {
		flag = TTEntry::EXACT;
	}

	int scoreToStore = best;
	if (scoreToStore > MATE)  scoreToStore += ply;
	if (scoreToStore < -MATE) scoreToStore -= ply;

	tt.store(boardState.hash, depth, scoreToStore, flag, bestMove);

	return best;
}

int bestScore;


Move Search::getBestMove(BoardState& boardState, SearchContext& ctx, int depth)
{
	nodesCount = 0;

	ctx.moveStack[0].count = 0;
	MoveGen::getLegalMoves(boardState, ctx.moveStack[0]);

	Move best = encodeMove(0, 0, EMPTY);
	bestScore = -INF;

	int alpha = -INF;
	int beta = INF;

	Move rootTtMove = 0;
	int dummyScore;
	tt.probe(boardState.hash, depth, alpha, beta, dummyScore, rootTtMove);

	for (int i = 0; i < ctx.moveStack[0].count; i++)
	{
		ctx.moveScores[0][i] = scoreMove(ctx.moveStack[0].moves[i], boardState, rootTtMove);
	}

	int result;
	for (int i = 0; i < ctx.moveStack[0].count; i++)
	{
		int bestMoveIndex = i;
		for (int j = i + 1; j < ctx.moveStack[0].count; j++)
		{
			if (ctx.moveScores[0][j] > ctx.moveScores[0][bestMoveIndex])
			{
				bestMoveIndex = j;
			}
		}

		std::swap(ctx.moveStack[0].moves[i], ctx.moveStack[0].moves[bestMoveIndex]);
		std::swap(ctx.moveScores[0][i], ctx.moveScores[0][bestMoveIndex]);

		MoveGen::makeMove(boardState, ctx.moveStack[0].moves[i], ctx.stateStack[0]);

		result = -negamax(boardState, ctx, depth - 1, 1, -beta, -alpha);

		MoveGen::unmakeMove(boardState, ctx.moveStack[0].moves[i], ctx.stateStack[0]);

		if (result > bestScore)
		{
			bestScore = result;
			best = ctx.moveStack[0].moves[i];
		}

		if (result > alpha)
		{
			alpha = result;
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
	return nodesCount;
}


