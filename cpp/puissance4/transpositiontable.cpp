#include <string.h>
#include "transpositiontable.h"

static TranspositionTable* instance = nullptr;

TranspositionTable* TranspositionTable::getInstance() {
    if (instance == nullptr) {
        instance = new TranspositionTable();
    }

    return instance;
}

TranspositionTable::TranspositionTable() {
    memset(table, 0xFF, sizeof(SEntry) * TABLE_SIZE);
}

uint64_t TranspositionTable::getHash(const Puissance4& p4, unsigned char player) const {
    uint64_t mask = 0, position = 0;
    uint64_t maskIdx = 1;

    for (int col=0;col<NB_COL;col++) {
        for (int row = 0;row<NB_ROW;row++) {
            unsigned char currentPlayer = p4.getCell(col, row);

            if (currentPlayer) {
                mask |= maskIdx;
                if (currentPlayer == player) {
                    position |= maskIdx;
                }
            }

            maskIdx <<= 1;
        }

        maskIdx <<= 1;
    }
    return mask + position;
}

int TranspositionTable::getScore(uint64_t hash, int depth) const {
    const SEntry* entry = &table[hash % TABLE_SIZE];
    if (entry->hash == hash) {
        if (entry->depth <= depth) {
            return entry->score;
        }
    }

    return P4INFINITY;
}

void TranspositionTable::setScore(uint64_t hash, int depth, int score) {
    auto idx = hash % TABLE_SIZE;
    table[idx].hash = hash;
    table[idx].depth = depth;
    table[idx].score = score;
}
