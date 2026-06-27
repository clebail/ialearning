#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include <cstdint>
#include "puissance4.h"

#define TABLE_SIZE                  1000000

class TranspositionTable {
public:
    typedef struct _SEntry {
        uint64_t hash;
        int depth;
        int score;

        /*_SEntry(uint64_t hash, int depth, int score) {
            this->hash = hash;
            this->depth = depth;
            this->score = score;
        }*/
    } SEntry;

    static TranspositionTable* getInstance();
    uint64_t getHash(const Puissance4& p4, unsigned char player) const;
    int getScore(uint64_t hash, int depth) const;
    void setScore(uint64_t hash, int depth, int score);
private:
    SEntry table[TABLE_SIZE];

    TranspositionTable();
};

#endif // TRANSPOSITIONTABLE_H
