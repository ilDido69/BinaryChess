#include "zobrist.h"
#include "tt.h"

TranspositionTable::TranspositionTable(size_t sizeInMB)
{
    allocate(sizeInMB);
}

void TranspositionTable::allocate(size_t sizeInMB)
{
    size_t bytes = sizeInMB * 1024 * 1024;
    size_t maxEntries = bytes / sizeof(TTEntry);

    size_t powerOf2 = 1;
    while (powerOf2 * 2 <= maxEntries) {
        powerOf2 *= 2;
    }

    nEntries = powerOf2;
    mask = nEntries - 1;

    table.resize(nEntries);
    clear();
}

void TranspositionTable::clear()
{
    std::fill(table.begin(), table.end(), TTEntry{ 0, 0, 0, TTEntry::LOWERBOUND, encodeMove(0, 0, EMPTY)});
}

bool TranspositionTable::probe(uint64_t key, int depth, int alpha, int beta, int& ttScore, Move& ttMove)
{
    size_t index = key & mask;
    const TTEntry& entry = table[index];

    if (entry.key == key) {
        ttMove = entry.bestMove;

        
        if (entry.depth >= depth) {
            ttScore = entry.score;

            if (entry.flag == TTEntry::EXACT) {
                return true;
            }
            if (entry.flag == TTEntry::UPPERBOUND && ttScore <= alpha) {
                return true;
            }
            if (entry.flag == TTEntry::LOWERBOUND && ttScore >= beta) {
                return true;
            }
        }
    }
    return false;
}


void TranspositionTable::store(uint64_t key, int depth, int score, TTEntry::Flag flag, Move move)
{
    size_t index = key & mask;
    if (table[index].key != key || table[index].depth < depth)
            table[index] = { key, depth, score, flag, move };
}
