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
    clear();
}

// 0xFF sur tout le tableau = des hash "impossibles" partout, donc aucun hit tant qu'on
// n'a rien réécrit. Même opération qu'à la construction, réutilisée à chaque relance.
void TranspositionTable::clear() {
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

bool TranspositionTable::probe(uint64_t hash, int depth, int &score, Bound &bound) const {
    const SEntry* entry = &table[hash % TABLE_SIZE];
    // Même position (et pas une simple collision d'index) ET calculée avec au moins
    // autant de profondeur restante : entry->depth ≤ depth ⇔ entrée plus proche de la
    // racine ⇔ sous-arbre au moins aussi profond que celui qu'on s'apprête à explorer.
    if (entry->hash == hash && entry->depth <= depth) {
        score = entry->score;
        // Inverse de store() : le mat est mémorisé en distance-AU-NŒUD ; on le ramène en
        // distance-à-la-racine COURANTE (re-soustraire/rajouter depth). Sans ça, la même
        // position lue à une autre profondeur annoncerait un mat à la mauvaise distance.
        if (score >= MATE_THRESHOLD) score -= depth;
        else if (score <= -MATE_THRESHOLD) score += depth;
        bound = entry->bound;
        return true;
    }

    return false;
}

void TranspositionTable::store(uint64_t hash, int depth, int score, Bound bound) {
    auto idx = hash % TABLE_SIZE;
    // Un score de mat encode une distance-à-la-RACINE (WIN_SCORE - depth). La TT survit
    // d'un coup à l'autre : la racine bouge, donc on neutralise cette dépendance en le
    // convertissant en distance-AU-NŒUD avant de l'écrire (probe() refait l'inverse).
    if (score >= MATE_THRESHOLD) score += depth;
    else if (score <= -MATE_THRESHOLD) score -= depth;
    table[idx].hash = hash;
    table[idx].depth = depth;
    table[idx].score = score;
    table[idx].bound = bound;
}
