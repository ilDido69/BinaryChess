#pragma once

#include "utils.h"
#include <bit>
#include <string>

namespace MoveGen {
    //generate moves for single pieces
    void generatePawnMoves(const BoardState& boardState, MoveList& out);
    void generateKnightMoves(const BoardState& boardState, MoveList& out);
    void generateBishopMoves(const BoardState& boardState, MoveList& out);
    void generateRookMoves(const BoardState& boardState, MoveList& out);
    void generateQueenMoves(const BoardState& boardState, MoveList& out);
    void generateKingMoves(const BoardState& boardState, MoveList& out);

    //generate moves for a player
    void getAllMoves(const BoardState& boardState, MoveList& out);      // pseudoLegal
    void getLegalMoves(BoardState& boardState, MoveList& out);    // legal

    //make and unmake a move
    void makeMove(BoardState& boardState, Move move, StateInfo& saved);
    void unmakeMove(BoardState& boardState, Move move, const StateInfo& saved);

    //utilities
    bool isAttacked(const BoardState& boardState, int sq, Color attacker);
    int  findKing(const BoardState& boardState, Color color);
    //used in moveGen, after making the move controls if the other side is in check
    bool isCheck(const BoardState& boardState);
    //if the moving side's king is in check
    bool onCheck(const BoardState& boardState);
    void resetBoardState(BoardState& boardState, SearchContext& ctx);
    void resetBoardState(BoardState& boardState, SearchContext& ctx, std::string fen);
    int getGameState(BoardState boardState); //for gui
}