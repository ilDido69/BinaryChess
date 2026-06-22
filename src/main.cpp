/*
Main:
You can choose between perft and game move (temporarily only perft because game.cpp and game.h need to be updated with the new generator).
*/

#include "utils.h"
#include "moveGen.h"
#include <string>
#include <iostream>
#include <chrono>
#include <map>

//used in printMove()
std::map<int, std::string> itos
{
    {0,  "a1"}, {1,  "b1"}, {2,  "c1"}, {3,  "d1"},
    {4,  "e1"}, {5,  "f1"}, {6,  "g1"}, {7,  "h1"},
    {8,  "a2"}, {9,  "b2"}, {10, "c2"}, {11, "d2"},
    {12, "e2"}, {13, "f2"}, {14, "g2"}, {15, "h2"},
    {16, "a3"}, {17, "b3"}, {18, "c3"}, {19, "d3"},
    {20, "e3"}, {21, "f3"}, {22, "g3"}, {23, "h3"},
    {24, "a4"}, {25, "b4"}, {26, "c4"}, {27, "d4"},
    {28, "e4"}, {29, "f4"}, {30, "g4"}, {31, "h4"},
    {32, "a5"}, {33, "b5"}, {34, "c5"}, {35, "d5"},
    {36, "e5"}, {37, "f5"}, {38, "g5"}, {39, "h5"},
    {40, "a6"}, {41, "b6"}, {42, "c6"}, {43, "d6"},
    {44, "e6"}, {45, "f6"}, {46, "g6"}, {47, "h6"},
    {48, "a7"}, {49, "b7"}, {50, "c7"}, {51, "d7"},
    {52, "e7"}, {53, "f7"}, {54, "g7"}, {55, "h7"},
    {56, "a8"}, {57, "b8"}, {58, "c8"}, {59, "d8"},
    {60, "e8"}, {61, "f8"}, {62, "g8"}, {63, "h8"}
};
std::array<std::string, 7> pieces = { "Pawn", "Knight", "Bishop", "Rook", "Queen", "King", "Empty" };

//used for debugging
void printMove(Move m)
{
    if (m > 4194303)
        std::cout << "Error : " << m << std::endl;
    else
    {
        Piece    moved = getMoved(m);
        int      from = getFrom(m);
        int      to = getTo(m);
        MoveFlag flag = getFlag(m);
        Piece    promo = getPromo(m);

        std::cout << "From: " << itos[from] << std::endl;
        std::cout << "To: " << itos[to] << std::endl;
        std::cout << "Piece: " << pieces[static_cast<int>(moved)] << std::endl;
    }
}

//0 = Game, 1 = perft
int inputMode()
{
    while (true)
    {
        std::string in;
        std::cout << "Select mode" << std::endl;
        std::cout << "0: Game,\n1: perft\nInput: ";
        std::cin >> in;
        if (in == "0")
            return 0;
        if (in == "1")
            return 1;
        std::cout << "Input error" << std::endl;
    }
}

//return depth
int inputDepth()
{
    std::string s;
    int maxDepth;
    while (true)
    {
        std::cout << "Depth (int): ";
        std::cin >> s;
        try
        {
            maxDepth = std::stoi(s);
            if (maxDepth > 0)
                break;
            std::cout << "Error: depth must be > 0" << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    return maxDepth;
}

//return fen, temporary it doesn't checks if it is valid, temporary "reset" -> startPos
std::string inputFen()
{
    std::string fen;
    std::cout << "Fen: ";
    std::cin.ignore();
    std::getline(std::cin, fen);
    return fen;
}

//stack for moveList and stateInfo
constexpr int MAX_PLY = 128;
MoveList moveListStack[MAX_PLY] = {};
StateInfo stateStack[MAX_PLY] = {};

//function that recursively counts all possible legal move sequences from a given position to a specified depth, used to verify move generation correctness and NPS.
uint64_t perft(BoardState& board, int depth, int ply = 0) {
    
    MoveList& moves = moveListStack[ply];
    moves.count = 0;
    MoveGen::getLegalMoves(board, moves);

    if (depth == 1) return moves.count;

    uint64_t nodes = 0;
    for (int i = 0; i < moves.count; i++) {
        MoveGen::makeMove(board, moves.moves[i], stateStack[ply]);
        nodes += perft(board, depth - 1, ply + 1);
        MoveGen::unmakeMove(board, moves.moves[i], stateStack[ply]);
    }
    return nodes;
}

uint64_t perftWithPrint(BoardState& board, int depth, int ply = 0) {

    if (depth == 0) return 1;
    MoveList& moves = moveListStack[ply];
    moves.count = 0;
    MoveGen::getLegalMoves(board, moves);

    uint64_t nodes = 0;
    for (int i = 0; i < moves.count; i++) {
        printMove(moves.moves[i]);
        MoveGen::makeMove(board, moves.moves[i], stateStack[ply]);
        nodes += perftWithPrint(board, depth - 1, ply + 1);
        MoveGen::unmakeMove(board, moves.moves[i], stateStack[ply]);
    }
    return nodes;
}

//main - temporarily only for perft
int main()
{

    while (true)
    {
        int maxDepth = inputDepth();
        BoardState boardState;
        std::string fen = inputFen();
        if (fen == "reset")
            MoveGen::resetBoardState(boardState);
        else
        {
            MoveGen::resetBoardState(boardState, fen);
            //MoveGen::makeMove(boardState, encodeMove(4, 6, KING, CASTLE_K), stateStack[0]);
        }

        for (int depth = 1; depth <= maxDepth; depth++)
        {
            auto start = std::chrono::high_resolution_clock::now();
            uint64_t nodes = perft(boardState, depth);
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            std::cout << "Depth " << depth
                << " | Nodi: " << nodes
                << " | Tempo: " << ms << "ms"
                << " | NPS: " << (uint64_t)(nodes / ms * 1000)
                << "\n";
        }
    }
    return 0;
}