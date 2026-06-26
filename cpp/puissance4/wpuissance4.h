#ifndef WPUISSANCE4_H
#define WPUISSANCE4_H

#include <QWidget>
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
protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
private:
    Puissance4 *board = nullptr;
    bool gameOver = false;   // gelé après une victoire / un match nul

    // Géométrie de la grille (taille de case + décalage de centrage), partagée
    // par le dessin et le mapping clic→colonne pour qu'ils ne dérivent jamais.
    int cellMetrics(int &offsetX, int &offsetY) const;
    // Colonne sous l'abscisse x du clic, ou -1 si hors grille.
    int columnAt(int x) const;
};

#endif // WPUISSANCE4_H
