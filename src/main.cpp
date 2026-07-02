/*
Main:
You can choose between perft and game move (temporarily only perft because game.cpp and game.h need to be updated with the new generator).
*/

#include "utils.h"
#include "moveGen.h"
#include "search.h"
#include "zobrist.h"
#include <string>
#include <iostream>
#include <chrono>
#include <map>

std::string squareToAlgebraic(int sq) {
    std::string s;
    s += ('a' + sq % 8);
    s += ('1' + sq / 8);
    return s;
}

int algebraicToSquare(std::string s) {
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    return rank * 8 + file;
}

//used for debugging
void printMove(Move m)
{
    int from = getFrom(m);
    int to = getTo(m);
    Piece promo = getPromo(m);

    std::string promoPiece = "";
    if (promo != EMPTY)
    {
        switch (promo)
        {
        case KNIGHT:
            promoPiece = "n";
            break;
        case BISHOP:
            promoPiece = "b";
            break;
        case ROOK:
            promoPiece = "r";
            break;
        case QUEEN:
            promoPiece = "q";
            break;
        }
    }
    std::cout << squareToAlgebraic(from) << squareToAlgebraic(to) << promoPiece << "\n" << std::flush;
}

//return depth if valid else -1
int validateDepth(std::string depth)
{
    int maxDepth;
    try
    {
        maxDepth = std::stoi(depth);
        if (maxDepth > 0)
            return maxDepth;
        std::cout << "Error: depth must be > 0" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
    return -1;
}

//return fen if valid else "error"
std::string validateFen(std::string fen)
{
    return fen;
}

//return move if valid else "error" 
std::string validateMove(std::string move)
{
    return move;
}

std::vector<std::string> splitMoves(std::string s)
{
    std::vector<std::string> moves;
    std::istringstream iss(s);
    std::string mv;
    while (iss >> mv)
        moves.push_back(mv);
    return moves;
}

UciCommand inputUci()
{
    while (true)
    {
        UciCommand cmd;
        std::string in;
        bool flag = false;
        std::getline(std::cin, in);
        if (in == "uci")
            cmd.type = UciType::UCI;
        else if (in == "isready")
            cmd.type = UciType::ISREADY;
        else if (in == "quit")
            cmd.type = UciType::QUIT;
        else if (in == "ucinewgame")
            cmd.type = UciType::NEWGAME;
        else if (in.starts_with("go depth "))
        {
            cmd.type = UciType::GODEPTH;
            int depth = validateDepth(in.substr(9));
            if (depth != -1)
                cmd.depth = depth;
            else
                continue;
        }
        else if (in == "position startpos" || in.starts_with("position startpos moves "))
        {
            cmd.type = UciType::POSITION;
            if (in.starts_with("position startpos moves "))
                cmd.moves = splitMoves(in.substr(24));
        }
        else if (in.starts_with("position fen "))
        {
            cmd.type = UciType::POSITION;
            std::string rest = in.substr(13);

            std::string fenPart = rest;
            auto movesPos = rest.find(" moves ");
            if (movesPos != std::string::npos)
            {
                fenPart = rest.substr(0, movesPos);
                cmd.moves = splitMoves(rest.substr(movesPos + 7));
            }

            std::string fen = validateFen(fenPart);
            if (fen != "error")
                cmd.fen = fen;
            else
                continue;
        }
        else
        {
            std::cout << "Unknown command '" << in << "'. Type 'help' for more information" << std::endl;
            flag = true;
        }
        if (!flag)
            return cmd;
    }
}

StateInfo uciMakeMoveStack;
void uciMakeMoves(BoardState& boardState, SearchContext& ctx, std::vector<std::string> moves)
{
    if (moves.empty())
        return;

    std::string move = moves[0];
    moves.erase(moves.begin());

    Color side = boardState.sideToMove;
    int from = algebraicToSquare(move.substr(0, 2));
    int to = algebraicToSquare(move.substr(2, 2));
    Piece piece = getPiece(boardState, side, from);
    MoveFlag flag = QUIET;
    Piece promo = EMPTY;
    if (move.size() == 5)
    {
        switch (move[4])
        {
        case 'q':
            promo = QUEEN;
            break;
        case 'r':
            promo = ROOK;
            break;
        case 'b':
            promo = BISHOP;
            break;
        case 'n':
            promo = KNIGHT;
            break;
        }
    }
    if (piece == KING)
    {
        if (from == 4 && side == WHITE || from == 60 && side == BLACK)
        {
            if (to == 6 && side == WHITE || to == 62 && side == BLACK)
                flag = CASTLE_K;
            else if (to == 2 && side == WHITE || to == 58 && side == BLACK)
                flag = CASTLE_Q;
        }
    }
    else if (piece == PAWN)
    {
        if (to / 8 == 7 && side == WHITE || to / 8 == 0 && side == BLACK)
            flag = PROMO;
        else if (from / 8 == 4 && side == WHITE || from / 8 == 3 && side == BLACK)
        {
            int d = to % 8 - from % 8;
            if (d == 1 || d == -1)
                flag = EN_PASSANT;
        }
    }
    if (boardState.byColor[~side] & (1ULL << to))
    {
        if (flag == PROMO)
            flag = PROMO_CAP;
        else
            flag = CAPTURE;
    }

    MoveGen::makeMove(boardState, encodeMove(from, to, piece, flag, promo), uciMakeMoveStack);
    ctx.pushHash(boardState.hash);
    uciMakeMoves(boardState, ctx, moves);
}

SearchContext ctx;

int main()
{
    Zobrist::init();

    BoardState boardState;
    MoveGen::resetBoardState(boardState, ctx);

    while (true)
    {
        UciCommand cmd = inputUci();
        switch (cmd.type)
        {
        case UciType::UCI:
            std::cout << "id name BinaryChess v1.0\n";
            std::cout << "id author ilDido69\n";
            std::cout << "uciok\n" << std::flush;
            break;
        case UciType::ISREADY:
            std::cout << "readyok\n" << std::flush;
            break;
        case UciType::QUIT:
            return 0;
        case UciType::NEWGAME:
            MoveGen::resetBoardState(boardState, ctx);
            break;
        case UciType::GODEPTH:
        {
            Move bestMove = Search::getBestMove(boardState, ctx, cmd.depth);
            std::cout << "info score cp " << Search::getScore() << "\n";
            std::cout << "bestmove ";
            printMove(bestMove);
            break;
        }
        case UciType::POSITION:
            if (cmd.fen == "")
                MoveGen::resetBoardState(boardState, ctx);
            else
                MoveGen::resetBoardState(boardState, ctx, cmd.fen);
            if (!cmd.moves.empty())
                uciMakeMoves(boardState, ctx, cmd.moves);
            break;
        }
    }
}

/*
//return only right commands
ShellCommand inputShell()
{
    while (true)
    {
        ShellCommand cmd;
        std::string in;
        bool flag = false;
        std::getline(std::cin, in);
        if (in == "quit")
            cmd.type = QUIT;
        else if (in == "help")
            cmd.type = HELP;
        else if (in == "settings")
            cmd.type = SETTINGS;
        else if (in == "play")
            cmd.type = PLAY;
        else if (in == "position startpos")
            cmd.type = POSITION;
        else if (in.starts_with("position fen "))
        {
            cmd.type = POSITION;
            std::string fen = validateFen(in.substr(13));
            if (fen != "error")
                cmd.fen = fen;
            else
                continue;
        }
        else if (in.starts_with("valuate depth "))
        {
            cmd.type = VALUATE;
            int depth = validateDepth(in.substr(14));
            if (depth != -1)
                cmd.depth = depth;
            else
                continue;
        }
        else if (in.starts_with("perft depth "))
        {
            cmd.type = PERFT;
            int depth = validateDepth(in.substr(12));
            if (depth != -1)
                cmd.depth = depth;
            else
                continue;
        }
        else if (in.starts_with("move "))
        {
            cmd.type = MOVE;
            std::string move = validateMove(in.substr(5));
            if (move != "error")
                cmd.move = move;
            else
                continue;
        }
        else
        {
            std::cout << "Unknown command '" << in << "'. Type 'help' for more information" << std::endl;
            flag = true;
        }
        if (!flag)
            return cmd;
    }
}


void printHelp()
{
    std::cout << "Commands:\n"
        << "  quit\n"
        << "  help\n"
        << "  settings\n"
        << "  play\n"
        << "  position startpos / fen <FEN>\n"
        << "  valuate depth <DEPTH>\n"
        << "  perft depth <DEPTH>\n"
        << "  move <MOVE>\n";
}

//function that recursively counts all possible legal move sequences from a given position to a specified depth, used to verify move generation correctness and NPS.
uint64_t perft(BoardState& board, int depth, int ply = 0) {
    
    MoveList& moves = ctx.moveStack[ply];
    moves.count = 0;
    MoveGen::getLegalMoves(board, moves);

    if (depth == 1) return moves.count;

    uint64_t nodes = 0;
    for (int i = 0; i < moves.count; i++) {
        MoveGen::makeMove(board, moves.moves[i], ctx.stateStack[ply]);
        nodes += perft(board, depth - 1, ply + 1);
        MoveGen::unmakeMove(board, moves.moves[i], ctx.stateStack[ply]);
    }
    return nodes;
}

uint64_t perftWithPrint(BoardState& board, int depth, int ply = 0) {

    if (depth == 0) return 1;
    MoveList& moves = ctx.moveStack[ply];
    moves.count = 0;
    MoveGen::getLegalMoves(board, moves);

    uint64_t nodes = 0;
    for (int i = 0; i < moves.count; i++) {
        printMove(moves.moves[i]);
        MoveGen::makeMove(board, moves.moves[i], ctx.stateStack[ply]);
        nodes += perftWithPrint(board, depth - 1, ply + 1);
        MoveGen::unmakeMove(board, moves.moves[i], ctx.stateStack[ply]);
    }
    return nodes;
}


//main - temporarily only for perft
int main()
{
    BoardState boardState;
    MoveGen::resetBoardState(boardState);

    while (true)
    {
        ShellCommand cmd = inputShell();
        switch (cmd.type)
        {
        case QUIT:
            return 0;
        case HELP:
            printHelp();
            break;
        case SETTINGS:
            std::cout << "work in progress\n";
            break;
        case PLAY:
        {
            Game game;
            while (game.running())
            {
                game.update();
                game.render();
            }
            break;
        }
        case POSITION:
            if (cmd.fen.empty())
                MoveGen::resetBoardState(boardState);
            else
                MoveGen::resetBoardState(boardState, cmd.fen);
            break;
        case VALUATE:
            printMove(Search::getBestMove(boardState, cmd.depth));
            break;
        case PERFT:
        {
            int maxDepth = cmd.depth;
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
            break;
        case MOVE:
            std::cout << "work in progress\n";
            break;
        }
    }
}

*/