#include <QPainter>
#include "wpuissance4.h"

WPuissance4::WPuissance4(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

WPuissance4::~WPuissance4() {
}

void WPuissance4::setBoard(Puissance4 *board) {
    this->board = board;
    repaint();
}

void WPuissance4::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(176, 182, 191));

    if (board == nullptr)
        return;

    // Cases carrées → ronds (jamais d'ovales) ; on prend le plus grand
    // côté qui tient à la fois en largeur et en hauteur, puis on centre.
    const int cell = qMin(width() / NB_COL, height() / NB_ROW);
    const int offsetX = (width() - cell * NB_COL) / 2;
    const int offsetY = (height() - cell * NB_ROW) / 2;
    const int margin = qMax(2, cell / 10);

    painter.setPen(Qt::NoPen);
    for (int row = 0; row < NB_ROW; row++) {
        for (int col = 0; col < NB_COL; col++) {
            unsigned char value = board->getCell(col, row);

            // row 0 = bas du plateau → dessiné en bas de la fenêtre
            int x = offsetX + col * cell;
            int y = offsetY + (NB_ROW - 1 - row) * cell;
            QRect disc(x + margin, y + margin,
                       cell - 2 * margin, cell - 2 * margin);

            painter.setBrush(value & 1 ? QColor(240, 138, 138)
                                       : (value & 2 ? QColor(130, 170, 230)
                                                    : QColor(245, 245, 245)));
            painter.drawEllipse(disc);
        }
    }
}
