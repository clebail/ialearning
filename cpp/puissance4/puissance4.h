#ifndef PUISSANCE4_H
#define PUISSANCE4_H

#define CELL_BIT_COUNT      2
#define NB_COL              7
#define NB_ROW              6
#define BOARD_SIZE          (NB_COL*NB_ROW)
#define PLAYER1             ((unsigned char)1)
#define PLAYER2             ((unsigned char)2)
#define PLAYERS             ((unsigned char)(PLAYER1+PLAYER2))
#define INFINITY            ((int)1000000)
#define MAX_DEPTH           7

class Puissance4 {
public:
    Puissance4();

    void reset();
    bool canPlay(int col) const;
    int availableColumns(int *a) const;
    unsigned char play(int col);
    unsigned char getCell(int col, int row) const;
    unsigned char win() const;
    bool isFull() const;
private:
    static const int COLUMNS_ORDER[NB_COL];

    unsigned short board[NB_ROW];
    unsigned char player;

    void setCell(int col, int row, unsigned char value);
    int minimax(int depth = 0, int alpha = INFINITY, int beta = -INFINITY) const;
    int evaluate() const;
};

#endif // PUISSANCE4_H
