#ifndef PUISSANCE4_H
#define PUISSANCE4_H

#define CELL_BIT_COUNT      2
#define NB_COL              7
#define NB_ROW              6
#define BOARD_SIZE          (NB_COL*NB_ROW)
#define PLAYER1             ((unsigned char)1)
#define PLAYER2             ((unsigned char)2)
#define PLAYERS             ((unsigned char)(PLAYER1+PLAYER2))
#define P4INFINITY          ((int)1000000)
#define MAX_DEPTH           11

class Puissance4 {
public:
    Puissance4();
    Puissance4(const Puissance4 &other) = default;
    Puissance4& operator=(const Puissance4 &other) = default;

    void reset();
    bool canPlay(int col) const;
    int availableColumns(int *a) const;
    // rowOut (optionnel) reçoit la ligne où le jeton s'est posé : utile au minimax
    // pour ne tester la victoire qu'autour de ce dernier coup.
    unsigned char play(int col, int *rowOut = nullptr);
    unsigned char getCell(int col, int row) const;
    unsigned char win() const;
    bool isFull() const;
    int bestMove() const;

    // Nœuds explorés par le DERNIER bestMove() (stats UI). Statique car minimax
    // récursif travaille sur des copies du plateau : seul un compteur partagé peut
    // agréger tout l'arbre. Remis à 0 en début de bestMove().
    static long nodes;
private:
    static const int COLUMNS_ORDER[NB_COL];

    unsigned short board[NB_ROW];
    unsigned char player;

    void setCell(int col, int row, unsigned char value);
    // Victoire incrémentale : seul le dernier coup peut créer un alignement, donc on
    // ne teste que les fenêtres passant par (lastCol, lastRow) au lieu des 69.
    unsigned char win(int lastCol, int lastRow) const;
    int minimax(int depth, int alpha, int beta, int lastCol, int lastRow) const;
    int evaluate() const;
};

#endif // PUISSANCE4_H
