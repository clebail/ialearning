#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include <cstdint>
#include "puissance4.h"

#define TABLE_SIZE                  1000000

class TranspositionTable {
public:
    // Nature du score mémorisé, vis-à-vis de la fenêtre αβ qui l'a produit :
    //  EXACT : vraie valeur minimax (fenêtre entièrement explorée) ;
    //  LOWER : borne INFÉRIEURE — une coupure β a tronqué la recherche, la vraie valeur
    //          est ≥ score (on s'est arrêté en sachant déjà que c'était « trop bon ») ;
    //  UPPER : borne SUPÉRIEURE — échec bas, la vraie valeur est ≤ score.
    // C'est ce drapeau qui corrige le bug : un score né d'un élagage n'est qu'une borne,
    // jamais une vérité, et ne doit être réutilisé que là où cette borne tranche.
    enum Bound : int { EXACT, LOWER, UPPER };

    typedef struct _SEntry {
        uint64_t hash;
        int depth;
        int score;
        Bound bound;
    } SEntry;

    static TranspositionTable* getInstance();
    // Vide toute la table : à appeler à chaque nouvelle partie (les scores en cache
    // dépendent de la profondeur et du mode d'évaluation courants).
    void clear();
    uint64_t getHash(const Puissance4& p4, unsigned char player) const;
    // Sonde la table : si une entrée fiable existe (même position, calculée avec au moins
    // autant de profondeur restante), remplit score+bound et renvoie true. C'est au
    // minimax de décider si la borne obtenue suffit à trancher la fenêtre courante.
    bool probe(uint64_t hash, int depth, int &score, Bound &bound) const;
    void store(uint64_t hash, int depth, int score, Bound bound);
private:
    SEntry table[TABLE_SIZE];

    TranspositionTable();
};

#endif // TRANSPOSITIONTABLE_H
