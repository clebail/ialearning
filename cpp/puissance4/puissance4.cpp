#include <string.h>
#include "puissance4.h"

Puissance4::Puissance4() {
    memset(board, 0, sizeof(unsigned char) * BOARD_SIZE);
}

unsigned char Puissance4::getCell(int col, int row)  {
    int bit = CELL_BIT_COUNT * (NB_COL * row + col);
}
