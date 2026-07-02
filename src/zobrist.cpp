#include "zobrist.h"

namespace Zobrist {
    uint64_t bbKeys[2][6][64];
    uint64_t castlingKeys[16];
    uint64_t enPassantKeys[8];
    uint64_t sideToMoveKey;

    void init()
    {
        std::mt19937_64 rng(0x1234567891234567ULL);
        std::uniform_int_distribution<uint64_t> dist;

        for (int color = WHITE; color <= BLACK; color++)
            for (int piece = PAWN; piece <= KING; piece++)
                for (int sq = 0; sq < 64; sq++)
                    bbKeys[color][piece][sq] = dist(rng);

        for (int i = 0; i < 16; i++)
            castlingKeys[i] = dist(rng);

        for (int file = 0; file < 8; file++)
            enPassantKeys[file] = dist(rng);

        sideToMoveKey = dist(rng);
    }


    uint64_t zobristHash(BoardState& boardState)
    {
        uint64_t hash = 0;
        
        hash ^= boardState.sideToMove;

        for (int color = WHITE; color <= BLACK; color++)
        {
            for (int piece = PAWN; piece <= KING; piece++)
            {
                uint64_t bb = boardState.bb[color][piece];
                while (bb)
                {
                    int sq = std::countr_zero(bb);
                    bb &= bb - 1;
                    hash ^= bbKeys[color][piece][sq];
                }
            }
        }

        hash ^= castlingKeys[boardState.castlingRights];

        if (boardState.enPassantSq != -1)
        {
            hash ^= enPassantKeys[boardState.enPassantSq % 8];
        }

        if (boardState.sideToMove == BLACK)
        {
            hash ^= sideToMoveKey;
        }

        return hash;
    }
}