#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi(this);

    wPuissance4->setBoard(&p4);
}

MainWindow::~MainWindow() {

}
