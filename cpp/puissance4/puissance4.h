#ifndef PUISSANCE4_H
#define PUISSANCE4_H

#define CELL_BIT_COUNT      2
#define NB_COL              7
#define NB_ROW              6
#define BOARD_SIZE          (CELL_BIT_COUNT*NB_COL*NB_ROW)

class Puissance4 {
public:
    Puissance4();

    unsigned char getCell(int col, int row);
private:
    unsigned char board[BOARD_SIZE];
};

#endif // PUISSANCE4_H
