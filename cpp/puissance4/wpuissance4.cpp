#include <QPainter>
#include "wpuissance4.h"

WPuissance4::WPuissance4(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

WPuissance4::~WPuissance4() {
}

void WPuissance4::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    painter.setPen(QPen(Qt::white));
    painter.setBrush(QBrush(Qt::white));
    painter.drawRect(rect());
}
