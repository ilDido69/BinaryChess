/*

#include <cstdint>
#include <array>
#include <cstdlib>





namespace LookupTables {
    extern uint64_t knightAttacks[64];
    extern uint64_t kingAttacks[64];
    extern uint64_t pawnAttacks[2][64];

    extern uint64_t rookRays[4][64];
    extern uint64_t bishopRays[4][64];

    void init();
}

*/

/*
For each piece, for each square of the board, it generates all the positions were the piece can go (pawn captures only, not pushes).
It is all done at compile time.
*/

#pragma once
#include <array>
#include <cstdint>

namespace LookupTables {

    //std::abs() isn't marked constexpr until C++23, so it can't be evaluated at compile-time inside constexpr functions in C++17/20
    constexpr int abs(int x) { return x < 0 ? -x : x; }


    //generate knight tables
    constexpr std::array<uint64_t, 64> generateKnightAttacks() {
        std::array<uint64_t, 64> table = {};
        constexpr std::array<int, 8> moves = { 17, 10, -6, -15, -17, -10, 6, 15 };
        for (int i = 0; i < 64; i++)
            for (int move : moves) {
                int target = i + move;
                if (target >= 0 && target < 64)
                    if (abs(target % 8 - i % 8) <= 2)
                        table[i] |= (1ULL << target);
            }
        return table;
    }

    //generate king tables
    constexpr std::array<uint64_t, 64> generateKingAttacks() {
        std::array<uint64_t, 64> table = {};
        constexpr std::array<int, 8> moves = { -9, -8, -7, -1, 1, 7, 8, 9 };
        for (int i = 0; i < 64; i++)
            for (int move : moves) {
                int target = i + move;
                if (target >= 0 && target < 64)
                    if (abs(target % 8 - i % 8) <= 1)
                        table[i] |= (1ULL << target);
            }
        return table;
    }

    //generate pawn tables
    constexpr std::array<std::array<uint64_t, 64>, 2> generatePawnAttacks() {
        std::array<std::array<uint64_t, 64>, 2> table = {};
        for (int i = 0; i < 64; i++) {
            if (i / 8 == 0 || i / 8 == 7) continue;
            int col = i % 8;
            if (col > 0) table[0][i] |= (1ULL << (i + 7));
            if (col < 7) table[0][i] |= (1ULL << (i + 9));
            if (col > 0) table[1][i] |= (1ULL << (i - 9));
            if (col < 7) table[1][i] |= (1ULL << (i - 7));
        }
        return table;
    }

    //generate rook tables
    constexpr std::array<std::array<uint64_t, 64>, 4> generateRookRays() {
        std::array<std::array<uint64_t, 64>, 4> table = {};
        constexpr std::array<int, 4> dirs = { -8, -1, 1, 8 };
        for (int i = 0; i < 64; i++)
            for (int k = 0; k < 4; k++) {
                int dir = dirs[k];
                for (int j = 1; j < 8; j++) {
                    int target = i + dir * j;
                    int prevPos = i + dir * (j - 1);
                    if (target < 0 || target >= 64) break;
                    if (abs(target % 8 - prevPos % 8) > 1) break;
                    table[k][i] |= (1ULL << target);
                }
            }
        return table;
    }

    //generate bishop tables
    constexpr std::array<std::array<uint64_t, 64>, 4> generateBishopRays() {
        std::array<std::array<uint64_t, 64>, 4> table = {};
        constexpr std::array<int, 4> dirs = { -9, -7, 7, 9 };
        for (int i = 0; i < 64; i++)
            for (int k = 0; k < 4; k++) {
                int dir = dirs[k];
                for (int j = 1; j < 8; j++) {
                    int target = i + dir * j;
                    int prevPos = i + dir * (j - 1);
                    if (target < 0 || target >= 64) break;
                    if (abs(target % 8 - prevPos % 8) > 1) break;
                    table[k][i] |= (1ULL << target);
                }
            }
        return table;
    }

    inline constexpr auto knightAttacks = generateKnightAttacks();
    inline constexpr auto kingAttacks = generateKingAttacks();
    inline constexpr auto pawnAttacks = generatePawnAttacks();
    inline constexpr auto rookRays = generateRookRays();
    inline constexpr auto bishopRays = generateBishopRays();
}