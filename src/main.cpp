#include "Game.h"
#include "utils.h"
#include "moveGenerator.h"
#include <chrono>
#include <iostream>
#include <string>

//0 = Game, 1 = perft
int input()
{
    while (true)
    {
        std::string in;
        std::cout << "0: Game,\n1: perft\nInput: ";
        std::cin >> in;
        if (in == "0")
            return 0;
        if (in == "1")
            return 1;
        std::cout << "Input error" << std::endl;
    }
}

uint64_t perft(BoardState state, int depth)
{
    if (depth == 0) return 1;

    std::vector<Move> moves = moveGenerator::getAllLegalMoves(state);
    uint64_t nodes = 0;

    for (const Move& move : moves)
    {
        BoardState next = state;
        moveGenerator::applyMove(next, move);
        nodes += perft(next, depth - 1);
    }
    return nodes;
}

int main()
{
    int testMode = input();
    if (testMode == 1)
    {
        BoardState state;
        state.board = {
            Piece::wR, Piece::wN, Piece::wB, Piece::wQ, Piece::wK, Piece::wB, Piece::wN, Piece::wR,
            Piece::wP, Piece::wP, Piece::wP, Piece::wP, Piece::wP, Piece::wP, Piece::wP, Piece::wP,
            Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty,
            Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty,
            Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty,
            Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty,
            Piece::bP, Piece::bP, Piece::bP, Piece::bP, Piece::bP, Piece::bP, Piece::bP, Piece::bP,
            Piece::bR, Piece::bN, Piece::bB, Piece::bQ, Piece::bK, Piece::bB, Piece::bN, Piece::bR
        };
        state.whiteToMove = true;
        state.passantTarget = -1;
        state.canCastle = { true, true, true, true };

        for (int depth = 1; depth <= 6; depth++)
        {
            auto start = std::chrono::high_resolution_clock::now();
            uint64_t nodes = perft(state, depth);
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            std::cout << "Depth " << depth
                << " | Nodi: " << nodes
                << " | Tempo: " << ms << "ms"
                << " | NPS: " << (uint64_t)(nodes / ms * 1000)
                << "\n";
        }
        return 0;
    }

    Game game;
    while (game.running())
    {
        game.update();
        game.render();
    }

    return 0;
}
