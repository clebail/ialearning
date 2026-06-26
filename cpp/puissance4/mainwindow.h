#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <ui_mainwindow.h>

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    Puissance4 p4;

    // Cumuls sur la partie en cours (remis à zéro à chaque nouvelle partie).
    long totalNodes = 0;
    double totalTime = 0.0;
};
#endif // MAINWINDOW_H
