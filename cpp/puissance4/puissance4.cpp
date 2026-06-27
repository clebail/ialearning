#include <string.h>
#include <array>
#include <QtDebug>
#include "puissance4.h"

const int Puissance4::COLUMNS_ORDER[NB_COL] = {3, 2, 4, 1, 5, 0, 6};

long Puissance4::nodes = 0;

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

// Une case appartient à au plus 13 fenêtres (les 2 cases centrales du plateau).
constexpr int MAX_FEN_PAR_CASE = 13;

// Pour une case donnée : les indices (dans FENETRES) des fenêtres qui la contiennent.
struct CaseFenetres { int nb; int idx[MAX_FEN_PAR_CASE]; };

// Index inverse FENETRES, figé au build lui aussi : à partir d'une case, retrouver
// directement ses fenêtres. C'est ce qui rend win(col,row) O(13) au lieu de O(69).
// Si une case dépassait MAX_FEN_PAR_CASE, l'écriture idx[nb++] sortirait du tableau
// → erreur de compilation (constexpr). La table est donc auto-vérifiée, comme FENETRES.
constexpr std::array<CaseFenetres, BOARD_SIZE> construitFenetresParCase() {
    std::array<CaseFenetres, BOARD_SIZE> parCase{};  // nb initialisé à 0 partout

    for (int n = 0; n < NB_FENETRE; n++) {
        for (const Cell &c : FENETRES[n].cases) {
            CaseFenetres &cf = parCase[c.col * NB_ROW + c.row];
            cf.idx[cf.nb++] = n;
        }
    }

    return parCase;
}

constexpr std::array<CaseFenetres, BOARD_SIZE> FENETRES_PAR_CASE = construitFenetresParCase();

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

unsigned char Puissance4::play(int col, int *rowOut) {
    if (!canPlay(col)) return 0;

    int row = 0;
    while (getCell(col, row) != 0) row++;
    setCell(col, row, player);
    if (rowOut) *rowOut = row;

    unsigned char oldPlayer = player;
    player = PLAYERS - player;

    return oldPlayer;
}

unsigned char Puissance4::win() const {
    for (const Fenetre &f : FENETRES) {
        unsigned char j = getCell(f.cases[0].col, f.cases[0].row);
        if (j == 0) continue;  // fenêtre vide sur sa 1re case -> pas un alignement

        if (j == getCell(f.cases[1].col, f.cases[1].row)
         && j == getCell(f.cases[2].col, f.cases[2].row)
         && j == getCell(f.cases[3].col, f.cases[3].row))
            return j;
    }

    return 0;
}

// Comme win() (balayage des 69 fenêtres) mais expose les 4 cases gagnantes, pour
// que l'UI puisse les surligner. Utilisé hors point chaud (fin de partie seulement).
bool Puissance4::winningLine(int cols[4], int rows[4]) const {
    for (const Fenetre &f : FENETRES) {
        unsigned char j = getCell(f.cases[0].col, f.cases[0].row);
        if (j == 0) continue;

        if (j == getCell(f.cases[1].col, f.cases[1].row)
         && j == getCell(f.cases[2].col, f.cases[2].row)
         && j == getCell(f.cases[3].col, f.cases[3].row)) {
            for (int i = 0; i < 4; i++) {
                cols[i] = f.cases[i].col;
                rows[i] = f.cases[i].row;
            }
            return true;
        }
    }

    return false;
}

// Variante incrémentale appelée par le minimax : on sait qu'un seul jeton vient
// d'être posé en (lastCol, lastRow), donc seules les fenêtres passant par cette case
// peuvent former un nouvel alignement. ≤ 13 fenêtres testées au lieu de 69.
unsigned char Puissance4::win(int lastCol, int lastRow) const {
    unsigned char j = getCell(lastCol, lastRow);
    if (j == 0) return 0;  // case vide -> aucun alignement possible ici

    const CaseFenetres &cf = FENETRES_PAR_CASE[lastCol * NB_ROW + lastRow];
    for (int k = 0; k < cf.nb; k++) {
        const Fenetre &f = FENETRES[cf.idx[k]];
        if (j == getCell(f.cases[0].col, f.cases[0].row)
         && j == getCell(f.cases[1].col, f.cases[1].row)
         && j == getCell(f.cases[2].col, f.cases[2].row)
         && j == getCell(f.cases[3].col, f.cases[3].row))
            return j;
    }

    return 0;
}

bool Puissance4::isFull() const {
    // Gravité : une colonne est pleine ssi sa case du HAUT est occupée. Le plateau
    // est donc plein ssi les 7 cases de la ligne du haut le sont — une SEULE lecture,
    // board[NB_ROW-1], au lieu de balayer les 6 lignes.
    const unsigned short top = board[NB_ROW - 1];

    // Chaque case = 2 bits (00 vide, 01 / 10 occupée — jamais 11). On replie les 2 bits
    // de chaque case sur son bit BAS : (top | top>>1) pose le bit bas du groupe dès qu'un
    // des deux bits est mis. Les 7 colonnes ont leur bit bas en 0,2,4,6,8,10,12 → masque
    // 0x1555. Plateau plein ⇔ ces 7 bits valent tous 1.
    return ((top | (top >> 1)) & 0x1555) == 0x1555;
}

int Puissance4::minimax(int depth, int alpha, int beta, int lastCol, int lastRow) const {
    nodes++;
    unsigned char winner = win(lastCol, lastRow);

    if (winner == PLAYER2) {
        return 10000 - depth;
    }

    if (winner == PLAYER1) {
        return -10000 + depth;
    }

    if (isFull()) {
        return 0;
    }

    if(depth >= MAX_DEPTH) return evaluate();

    bool maximize = player == PLAYER2;
    int best = maximize ? -P4INFINITY : P4INFINITY;
    int cols[NB_COL];
    int nbAC = availableColumns(cols);


    for (int c=0;c<nbAC;c++) {
        Puissance4 other(*this);

        int row;
        other.play(cols[c], &row);

        int score = other.minimax(depth + 1, alpha, beta, cols[c], row);
        if (maximize) {
            best = qMax(best, score);
            alpha = qMax(alpha, best);
        } else {
            best = qMin(best, score);
            beta = qMin(beta, best);
        }

        if (alpha >= beta) {
            break;
        }
    }

    return best;
}

int Puissance4::evaluate() const {
    int score = 0;

    for (const Fenetre &f : FENETRES) {
        int nbX = 0, nbO = 0;
        for (const auto &[x, y] : f.cases) {
            unsigned char c = getCell(x, y);

            if (c == PLAYER2){
                nbX++;
            } else if (c == PLAYER1) {
                nbO++;
            }
        }
        if (nbX && nbO) continue;            // fenêtre morte : les deux camps y sont présents

        if (nbX == 3) {
            score += 1000;
        } else if (nbX == 2) {
            score += 100;
        } else if (nbO == 3) {
            score -= 1000;
        } else if (nbO == 2) {
            score -= 100;
        }
    }

    // Bonus centre : un pion au milieu est plus flexible (dans le plus de fenêtres).
    // Petit poids → ne départage qu'à menaces égales (typiquement en début de partie).
    for (int y = 0; y < NB_ROW; y++) {
        unsigned char c = getCell(COLONNE_CENTRE, y);
        if (c == PLAYER2) {
            score += BONUS_CENTRE;
        } else if (c == PLAYER1) {
            score -= BONUS_CENTRE;
        }
    }

    return score;
}

int Puissance4::bestMove() const {
    nodes = 0;   // compteur neuf pour ce coup
    bool maximize = player == PLAYER2;
    int best = maximize ? -P4INFINITY : P4INFINITY;
    int move = -1;
    int alpha = -P4INFINITY, beta = P4INFINITY;
    int cols[NB_COL];
    int nbAC = availableColumns(cols);

    for (int c=0;c<nbAC;c++) {
        Puissance4 other(*this);

        int row;
        other.play(cols[c], &row);

        int score = other.minimax(0, alpha, beta, cols[c], row);
        if (maximize ? score > best : score < best) {
            best = score;
            move = cols[c];
        }

        if (maximize) {
            alpha = qMax(alpha, best);
        } else {
            beta = qMin(beta, best);
        }
    }

    return move;
}
