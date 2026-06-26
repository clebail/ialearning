#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi(this);

    wPuissance4->setBoard(&p4);
    connect(rejouerButton, &QPushButton::clicked, wPuissance4, &WPuissance4::reset);
}

MainWindow::~MainWindow() {

}
