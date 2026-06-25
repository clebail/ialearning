#ifndef WPUISSANCE4_H
#define WPUISSANCE4_H

#include <QWidget>
#include <ui_wpuissance4.h>

class WPuissance4 : public QWidget, private Ui::WPuissance4
{
    Q_OBJECT

public:
    explicit WPuissance4(QWidget *parent = nullptr);
    ~WPuissance4();
protected:
    virtual void paintEvent(QPaintEvent *event);
};

#endif // WPUISSANCE4_H
