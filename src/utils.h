#pragma once

#include <cstdint>
#include <array>

enum class Piece : uint8_t{
	wK, wQ, wR, wB, wN, wP,
	bK , bQ, bR, bB, bN, bP,
	Empty
};

enum class MoveFlag : uint8_t {
	None, EnPassant, Promotion, Castle
};

enum class Colors : uint8_t {
	White, Black, Draw, Continue
};

struct Move {
	int from;
	int to;
	Piece promotionPiece = Piece::Empty;

	MoveFlag moveFlag = MoveFlag::None;
};

struct BoardState {
	std::array<Piece, 64> board;

	std::array<bool, 4> canCastle; //KQkq

	int passantTarget = -1;

	bool whiteToMove;
};

inline bool isWhite(Piece p) { return static_cast<int>(p) < 6; }
inline bool isBlack(Piece p) { return static_cast<int>(p) >= 6 && p != Piece::Empty; }
inline bool isCurrentTurn(Piece p, bool whiteToMove) { return whiteToMove ? isWhite(p) : isBlack(p); }