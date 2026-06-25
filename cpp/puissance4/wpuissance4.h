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
protected:
    virtual void paintEvent(QPaintEvent *event);
private:
    Puissance4 *board = nullptr;
};

#endif // WPUISSANCE4_H
