#include <string.h>
#include <QtDebug>
#include "puissance4.h"

const int Puissance4::COLUMNS_ORDER[NB_COL] = {3, 2, 4, 1, 5, 0, 6};

Puissance4::Puissance4() {
    memset(board, 0, sizeof(unsigned short) * NB_ROW);
    player = 1;
}

unsigned char Puissance4::getCell(int col, int row) const  {
    if (row >= 0 && row < NB_ROW && col >= 0 && col < NB_COL) {
        int decal = CELL_BIT_COUNT * (NB_COL - col - 1);
        unsigned short value = (board[row] >> decal) & 0x0003;

        return (unsigned char)value;

    }

    return 0;
}

void Puissance4::setCell(int col, int row, unsigned char value) {
    if (row >= 0 && row < NB_ROW && col >= 0 && col < NB_COL) {
        int decal = CELL_BIT_COUNT * (NB_COL - col - 1);
        unsigned short newValue = ((unsigned short)(value & 0x03) << decal);
        board[row] = board[row] & ~newValue;
        board[row] |= newValue;
    }
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
    player = 3 - player;

    return oldPlayer;
}