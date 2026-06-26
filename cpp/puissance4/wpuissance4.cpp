#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
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

void WPuissance4::reset() {
    if (board == nullptr)
        return;

    board->reset();
    gameOver = false;
    repaint();
}

// Cases carrées → ronds (jamais d'ovales) : on prend le plus grand côté qui tient
// à la fois en largeur et en hauteur, puis on centre la grille dans le widget.
int WPuissance4::cellMetrics(int &offsetX, int &offsetY) const {
    const int cell = qMin(width() / NB_COL, height() / NB_ROW);
    offsetX = (width() - cell * NB_COL) / 2;
    offsetY = (height() - cell * NB_ROW) / 2;
    return cell;
}

int WPuissance4::columnAt(int x) const {
    int offsetX, offsetY;
    const int cell = cellMetrics(offsetX, offsetY);
    if (cell <= 0 || x < offsetX)
        return -1;

    const int col = (x - offsetX) / cell;
    return col < NB_COL ? col : -1;
}

void WPuissance4::mouseReleaseEvent(QMouseEvent *event) {
    if (board == nullptr || gameOver)
        return;

    const int col = columnAt(event->pos().x());
    if (col < 0 || !board->canPlay(col))
        return;   // clic hors grille ou colonne pleine : on ignore

    const unsigned char who = board->play(col);
    repaint();

    if (board->win()) {
        gameOver = true;
        QMessageBox::information(this, "Puissance 4",
            QStringLiteral("Le joueur %1 gagne !").arg(who));
        return;
    }

    // Plus aucune colonne jouable et pas de gagnant → match nul.
    int colonnes[NB_COL];
    if (board->availableColumns(colonnes) == 0) {
        gameOver = true;
        QMessageBox::information(this, "Puissance 4", QStringLiteral("Match nul !"));
    }
}

void WPuissance4::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(176, 182, 191));

    if (board == nullptr)
        return;

    int offsetX, offsetY;
    const int cell = cellMetrics(offsetX, offsetY);
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
