/*
That file contains all the struct, enum and general functions.
*/

#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <string>

enum Piece : uint8_t{
	PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, EMPTY
};

enum GuiPiece : uint8_t {
	wK, wQ, wR, wB, wN, wP,
	bK, bQ, bR, bB, bN, bP,
	Empty
};

enum MoveFlag : uint8_t {
	QUIET = 0, CAPTURE, EN_PASSANT, CASTLE_K, CASTLE_Q, PROMO, PROMO_CAP
};

enum Color : uint8_t {
	WHITE, BLACK
};

enum class ShellType {
	QUIT, HELP, SETTINGS, PLAY,
	POSITION, VALUATE, PERFT, MOVE
};

enum class UciType {
	UCI, ISREADY, NEWGAME, POSITION, GODEPTH, QUIT
};

constexpr Color operator~(Color c) {
	return static_cast<Color>(c ^ 1);
}

using Move = uint32_t;

// Encode a move into uint32_t
constexpr Move encodeMove(int from, int to, Piece moved, MoveFlag flag = QUIET, Piece promo = EMPTY) 
{
	return static_cast<uint32_t>(from)
		| (static_cast<uint32_t>(to) << 6)
		| (static_cast<uint32_t>(flag) << 12)
		| (static_cast<uint32_t>(moved) << 16)
		| (static_cast<uint32_t>(promo) << 19);
}

// Extract data from a move
constexpr int getFrom(Move m) { return m & 0x3F; }
constexpr int getTo(Move m) { return (m >> 6) & 0x3F; }
constexpr MoveFlag getFlag(Move m) { return static_cast<MoveFlag>((m >> 12) & 0xF); }
constexpr Piece getMoved(Move m) { return static_cast<Piece>((m >> 16) & 0x7); }
constexpr Piece getPromo(Move m) { return static_cast<Piece>((m >> 19) & 0x7); }

struct MoveList {
	std::array<Move, 256> moves;
	int count = 0;
	void push(Move m) { moves[count++] = m; }

	void remove(int i) { moves[i] = moves[--count]; }
};

struct StateInfo {
	int enPassantSquare;
	uint8_t castlingRights; //KQkq
	int rule50;
	Piece capturedPiece;
};

struct BoardState {
	uint64_t bb[2][6] = {};
	uint64_t byColor[2] = {};
	uint64_t occupied = 0;

	Color sideToMove;
	int enPassantSq = -1;
	uint8_t castlingRights = 0xF;
	int rule50 = 0;
};

inline void clearBit(uint64_t& bb, int sq) { bb &= ~(1ULL << sq); }
inline void setBit(uint64_t& bb, int sq) { bb |= (1ULL << sq); }

inline Piece getPiece(BoardState& boardState, Color color, int sq)
{
	for (int piece = PAWN; piece <= KING; piece++)
		if (boardState.bb[color][piece] & (1ULL << sq))
			return static_cast<Piece>(piece);
	return EMPTY;
}

struct ShellCommand {
	ShellType type;
	int depth = 0;
	std::string move;
	std::string fen;        
};

struct UciCommand {
	UciType type;
	int depth = 0;
	std::vector<std::string> moves;
	std::string fen;
};