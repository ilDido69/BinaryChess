#pragma once

#include <cstdint>
#include <random>
#include <bit>
#include "utils.h"

namespace Zobrist {
    extern uint64_t bbKeys[2][6][64];
    extern uint64_t castlingKeys[16];
    extern uint64_t enPassantKeys[8];
    extern uint64_t sideToMoveKey;

    void init();

    uint64_t zobristHash(BoardState& boardState);
}