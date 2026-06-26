#include <string.h>
#include <array>
#include <QtDebug>
#include "puissance4.h"

const int Puissance4::COLUMNS_ORDER[NB_COL] = {3, 2, 4, 1, 5, 0, 6};

namespace {

// Une case du plateau (sert aussi de vecteur direction : delta colonne / ligne).
struct Cell { int col; int row; };
// Une fenêtre = 4 cases alignées.
struct Fenetre { Cell cases[4]; };

// Nombre canonique d'alignements de 4 au Puissance 4 7x6 (24 H + 21 V + 12 + 12).
constexpr int NB_FENETRE = 69;

// Géométrie FIGÉE du plateau : les 69 fenêtres de 4 cases alignées.
// Dans la version JS, ce triple balayage (colonnes x lignes x directions) tournait
// des millions de fois par coup (relancé à chaque feuille par evaluate). Marqué
// constexpr, le compilateur l'exécute UNE fois, AU BUILD : le binaire contient le
// tableau déjà figé, zéro calcul à l'exécution — pas même à l'init.
constexpr std::array<Fenetre, NB_FENETRE> construitFenetres() {
    // Un seul sens par orientation suffit : chaque fenêtre démarrant depuis chaque
    // case, balayer "->" couvre déjà les lectures droite-à-gauche.
    // horizontale, verticale, diagonale montante, diagonale descendante
    constexpr Cell DIRECTIONS[4] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    std::array<Fenetre, NB_FENETRE> fenetres{};
    int n = 0;

    for (int col = 0; col < NB_COL; col++) {
        for (int row = 0; row < NB_ROW; row++) {
            for (const Cell &d : DIRECTIONS) {
                // La fenêtre sort-elle du plateau ? Il suffit de tester sa dernière case.
                int finCol = col + d.col * 3;
                int finRow = row + d.row * 3;
                if (finCol < 0 || finCol >= NB_COL || finRow < 0 || finRow >= NB_ROW)
                    continue;

                for (int i = 0; i < 4; i++)
                    fenetres[n].cases[i] = { col + d.col * i, row + d.row * i };
                n++;
            }
        }
    }

    // Validation AU BUILD (l'équivalent compile-time du "recompter 69") :
    // - trop de fenêtres -> dépassement de std::array -> erreur de compilation ;
    // - trop peu -> ce throw est atteint -> évaluation non-constante -> erreur.
    // En géométrie correcte, n == 69, le throw n'est jamais atteint.
    if (n != NB_FENETRE)
        throw "construitFenetres: compte de fenetres incorrect";

    return fenetres;
}

// La table figée, calculée à la compilation.
constexpr std::array<Fenetre, NB_FENETRE> FENETRES = construitFenetres();

}  // namespace

Puissance4::Puissance4() {
    reset();
}

// Remet le plateau à vide, le joueur 1 reprend la main : partie neuve.
void Puissance4::reset() {
    memset(board, 0, sizeof(unsigned short) * NB_ROW);
    player = 1;
}

unsigned char Puissance4::getCell(int col, int row) const  {
    // Précondition : indices dans le plateau. Tous les appelants la garantissent
    // (canPlay valide col, play borne row, FENETRES est in-bounds par construction).
    // Q_ASSERT s'efface en release → zéro coût sur ce point chaud du minimax.
    Q_ASSERT(col >= 0 && col < NB_COL && row >= 0 && row < NB_ROW);

    int decal = CELL_BIT_COUNT * (NB_COL - col - 1);
    unsigned short value = (board[row] >> decal) & 0x0003;

    return (unsigned char)value;
}

void Puissance4::setCell(int col, int row, unsigned char value) {
    Q_ASSERT(col >= 0 && col < NB_COL && row >= 0 && row < NB_ROW);

    int decal = CELL_BIT_COUNT * (NB_COL - col - 1);
    unsigned short newValue = ((unsigned short)(value & 0x03) << decal);
    board[row] = board[row] & ~newValue;
    board[row] |= newValue;
}

bool Puissance4::canPlay(int col) const {
    return col >= 0 && col < NB_COL && getCell(col, NB_ROW - 1) == 0;
}

int Puissance4::availableColumns(int *a) const {
    int nb = 0;
    for (int i = 0;i < NB_COL;i++) {
        int col = COLUMNS_ORDER[i];
        if (canPlay(col)) {
            a[nb++] = col;
        }
    }

    return nb;
}

unsigned char Puissance4::play(int col) {
    if (!canPlay(col)) return 0;

    int row = 0;
    while (getCell(col, row) != 0) row++;
    setCell(col, row, player);

    unsigned char oldPlayer = player;
    player = PLAYERS - player;

    return oldPlayer;
}

bool Puissance4::win() const {
    for (const Fenetre &f : FENETRES) {
        unsigned char j = getCell(f.cases[0].col, f.cases[0].row);
        if (j == 0) continue;  // fenêtre vide sur sa 1re case -> pas un alignement

        if (j == getCell(f.cases[1].col, f.cases[1].row)
         && j == getCell(f.cases[2].col, f.cases[2].row)
         && j == getCell(f.cases[3].col, f.cases[3].row))
            return true;
    }

    return false;
}
