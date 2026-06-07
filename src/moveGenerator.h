#pragma once

#include <vector>
#include <array>
#include "utils.h"

/*
Class with functions that get moves or modify the boardState

Future improvement:
- Filter legal moves (before returning a possible move generate all the moves for the other color and find if someone attacks the square (the king)),
	and add it for checks, castle
- If a king or the rook moves they can't castle - if the moves were the king "walk" to castle are attacked he can't castle
- Create a function that returns if the game is ended (w - d - b) or not
- Create undoMove functions - use in getLegalMoves()
- Using array to improve the performaces without dynamic memory allocation
- Improve isSquareAttacked function (instead of generate all the moves and find if one attacks the baseSquare, start from the baseSquare and generate
	the squares that can attack the baseSquare and see if there is a piece on that squares
- Using bitboards to represent the boardstate and to generate moves using precompilated tables (create for all the squares for all the pieces 
	where it can move)
*/

class moveGenerator
{
private:
	//return true if the new position is in the board and is empty or of the other color
	static bool validTarget(const BoardState& boardState, int from, int to, int delta);
	//return all the possible move (legal and not)
	static std::vector<Move> getKingMoves(const BoardState& boardState, int square, bool castle = true);
	static std::vector<Move> getQueenMoves(const BoardState& boardState, int square);
	static std::vector<Move> getRookMoves(const BoardState& boardState, int square);
	static std::vector<Move> getBishopMoves(const BoardState& boardState, int square);
	static std::vector<Move> getKnightMoves(const BoardState& boardState, int square);
	static std::vector<Move> getPawnMoves(const BoardState& boardState, int square);

public:
	//find the king in the current position
	static int findKing(const BoardState& boardState, bool white);
	//generate all move for find if someone attacks it
	static bool isSquareAttacked(const BoardState& boardState, int square);
	//apply a move to the current boardstate
	static BoardState applyMove(BoardState& boardState, const Move& move);
	//return all the moves for a piece on the board  (pseudoLegal)
	static std::vector<Move> getPseudoLegalMoves(const BoardState& boardState, int square);
	//return only legal move for a piece on the board
	static std::vector<Move> getLegalMoves(const BoardState& boardState, int square);
	//return all the moves for a player (pseudoLegal)
	static std::vector<Move> getAllMoves(const BoardState& boardState);
	//return all the legal moves for a player
	static std::vector<Move> getAllLegalMoves(const BoardState& boardState);
	static Colors getGameState(BoardState& boardState);
};

