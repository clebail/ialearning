#ifndef WPUISSANCE4_H
#define WPUISSANCE4_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <ui_wpuissance4.h>
#include "puissance4.h"

class WPuissance4 : public QWidget, private Ui::WPuissance4
{
    Q_OBJECT

public:
    explicit WPuissance4(QWidget *parent = nullptr);
    ~WPuissance4();

    void setBoard(Puissance4 *board);
public slots:
    void reset();   // partie neuve : vide le plateau et dégèle les clics
    void aiStart(); // fait jouer l'IA en premier (sur plateau vide)
signals:
    // Émis après chaque coup de l'IA : nœuds explorés + temps de réflexion (ms).
    void aiMoved(long nodes, double timeMs);
    // Émis quand une nouvelle partie commence (pour remettre les cumuls à zéro).
    void statsReset();
protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
private:
    Puissance4 *board = nullptr;
    bool gameOver = false;   // gelé après une victoire / un match nul

    // Animation : la dernière boule jouée pulse (10 frames petit, 10 frames grand).
    int lastCol = -1, lastRow = -1;   // case animée (-1 = aucune)
    QTimer pulseTimer;                // cadence le rafraîchissement (une frame par tick)
    int pulseFrame = 0;              // frame courante de la pulsation
    void startPulse(int col, int row);

    // Fin de partie : les 4 cases de l'alignement gagnant, marquées d'une croix.
    bool hasWin = false;
    int winCols[4], winRows[4];

    // Géométrie de la grille (taille de case + décalage de centrage), partagée
    // par le dessin et le mapping clic→colonne pour qu'ils ne dérivent jamais.
    int cellMetrics(int &offsetX, int &offsetY) const;
    // Colonne sous l'abscisse x du clic, ou -1 si hors grille.
    int columnAt(int x) const;
    // Joue un coup de l'IA (chrono + bestMove + tests fin de partie). Sans effet
    // si la partie est finie : sert au tour normal comme au tout premier coup.
    void aiTurn();

    // Journal des coups de la partie, colonnes 1-indexées et collées (ex. "443"),
    // prêt à passer à solver-analyse.sh. Émis sur qDebug à chaque coup, humain comme
    // IA ; vidé à chaque nouvelle partie. N'a aucun effet sur le jeu (debug seul).
    QString moveSeq;
    void logMove(int col);
};

#endif // WPUISSANCE4_H
