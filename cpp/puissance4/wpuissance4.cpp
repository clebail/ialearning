#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QDebug>
#include "wpuissance4.h"

namespace {
// Pulsation de la dernière boule : 10 frames "petit" puis 10 frames "grand".
constexpr int PULSE_PHASE_FRAMES = 10;                       // frames par état
constexpr int PULSE_TOTAL_FRAMES = 2 * PULSE_PHASE_FRAMES;   // 10 petits + 10 grands
constexpr double PULSE_SMALL = 0.6;                          // échelle du disque "petit"
constexpr double PULSE_LARGE = 1.0;                          // échelle du disque "grand"
}  // namespace

WPuissance4::WPuissance4(QWidget *parent) : QWidget(parent) {
    setupUi(this);

    // Un tick = une frame ; après PULSE_TOTAL_FRAMES on arrête et la boule
    // reprend sa taille normale.
    pulseTimer.setInterval(16);
    connect(&pulseTimer, &QTimer::timeout, this, [this] {
        pulseFrame++;
        if (pulseFrame >= PULSE_TOTAL_FRAMES) {
            pulseTimer.stop();
            lastCol = lastRow = -1;
        }
        update();
    });
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
    hasWin = false;
    pulseTimer.stop();
    lastCol = lastRow = -1;
    moveSeq.clear();
    emit statsReset();
    repaint();
}

// Ajoute le coup au journal (colonne 0-indexée en interne → +1 pour le solveur) et
// affiche la séquence complète : la dernière ligne de qDebug se colle telle quelle
// dans solver-analyse.sh.
void WPuissance4::logMove(int col) {
    moveSeq += QString::number(col + 1);
    qDebug().noquote() << "Séquence des coups :" << moveSeq;
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

    // Pas d'animation sur le coup humain : bestMove() bloque ensuite la boucle
    // d'événements (le timer ne tournerait pas) → la boule resterait figée petite.
    // Seul le coup de l'IA est animé.
    unsigned char who = board->play(col);
    logMove(col);
    repaint();

    if (board->winningLine(winCols, winRows)) {
        gameOver = true;
        hasWin = true;
        repaint();   // dessine les croix avant la boîte de dialogue modale
        QMessageBox::information(this, "Fin de partie",
            QStringLiteral("Le joueur %1 gagne !").arg(int(who)));
        return;
    }

    // Plus aucune colonne jouable et pas de gagnant → match nul.
    int colonnes[NB_COL];
    if (board->availableColumns(colonnes) == 0) {
        gameOver = true;
        QMessageBox::information(this, "Fin de partie", QStringLiteral("Match nul !"));
        return;
    }

    aiTurn();
}

void WPuissance4::aiStart() {
    if (board == nullptr || gameOver)
        return;

    // Ouverture forcée : sur plateau vide le centre est le meilleur coup connu.
    // On le joue directement, sans lancer le minimax (réflexion inutile et coûteuse).
    int row;
    board->play(COLONNE_CENTRE, &row);
    logMove(COLONNE_CENTRE);
    startPulse(COLONNE_CENTRE, row);
    repaint();
}

void WPuissance4::startPulse(int col, int row) {
    lastCol = col;
    lastRow = row;
    pulseFrame = 0;       // repart de zéro même si une pulsation est en cours (jeu rapide)
    pulseTimer.start();   // relance le timer s'il tournait déjà
}

void WPuissance4::aiTurn() {
    if (board == nullptr || gameOver)
        return;

    // Tour de l'IA : on chronomètre la réflexion (bestMove) et on relève le
    // nombre de nœuds explorés, puis on remonte ça aux stats via aiMoved.
    QElapsedTimer timer;
    timer.start();
    const int aiCol = board->bestMove();
    const double timeMs = timer.nsecsElapsed() / 1.0e6;   // ns -> ms (haute précision)
    emit aiMoved(Puissance4::nodes, timeMs);

    int row;
    unsigned char who = board->play(aiCol, &row);
    logMove(aiCol);
    startPulse(aiCol, row);
    repaint();

    if (board->winningLine(winCols, winRows)) {
        gameOver = true;
        hasWin = true;
        repaint();   // dessine les croix avant la boîte de dialogue modale
        QMessageBox::information(this, "Fin de partie",
                                 QStringLiteral("Le joueur %1 gagne !").arg(int(who)));
        return;
    }

    // Plus aucune colonne jouable et pas de gagnant → match nul.
    int colonnes[NB_COL];
    if (board->availableColumns(colonnes) == 0) {
        gameOver = true;
        QMessageBox::information(this, "Fin de partie", QStringLiteral("Match nul !"));
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
            QRectF disc(x + margin, y + margin,
                        cell - 2 * margin, cell - 2 * margin);

            // Dernière boule jouée : 6 frames petit, 6 frames grand, tant que le timer tourne.
            if (col == lastCol && row == lastRow && pulseTimer.isActive()) {
                const bool grand = (pulseFrame / PULSE_PHASE_FRAMES) % 2 == 1;
                const double scale = grand ? PULSE_LARGE : PULSE_SMALL;
                const QPointF c = disc.center();
                disc.setSize(disc.size() * scale);
                disc.moveCenter(c);
            }

            painter.setBrush(value & 1 ? QColor(240, 138, 138)
                                       : (value & 2 ? QColor(130, 170, 230)
                                                    : QColor(245, 245, 245)));
            painter.drawEllipse(disc);
        }
    }

    // Alignement gagnant : une croix (X) au centre de chacune des 4 boules.
    if (hasWin) {
        QPen pen(QColor(40, 40, 40));
        pen.setWidth(qMax(3, cell / 7));   // trait plus gras
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);

        const int r = qRound((cell - 2 * margin) * 0.16);   // demi-diagonale du X (plus petite)
        for (int i = 0; i < 4; i++) {
            const int cx = offsetX + winCols[i] * cell + cell / 2;
            const int cy = offsetY + (NB_ROW - 1 - winRows[i]) * cell + cell / 2;
            painter.drawLine(cx - r, cy - r, cx + r, cy + r);
            painter.drawLine(cx - r, cy + r, cx + r, cy - r);
        }
    }
}
