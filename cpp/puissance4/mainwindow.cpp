#include <QPushButton>
#include <QLocale>
#include "mainwindow.h"

namespace {

// Remplace UNIQUEMENT la valeur (après <br/>) d'un label de stats, en gardant le
// libellé défini dans le .ui comme source unique → pas de texte dupliqué ici.
void setStatValue(QLabel *label, const QString &value) {
    QString text = label->text();
    const int br = text.indexOf(QStringLiteral("<br/>"));
    if (br >= 0)
        text = text.left(br);
    label->setText(text + QStringLiteral("<br/><b>") + value + QStringLiteral("</b>"));
}

}  // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi(this);

    wPuissance4->setBoard(&p4);
    connect(rejouerButton, &QPushButton::clicked, wPuissance4, &WPuissance4::reset);

    // Profondeur : fixe, posée une fois pour toutes.
    setStatValue(statDepth, QString::number(MAX_DEPTH));

    // Stats par coup + cumuls, alimentés à chaque coup de l'IA.
    connect(wPuissance4, &WPuissance4::aiMoved, this, [this](long nodes, double timeMs) {
        totalNodes += nodes;
        totalTime += timeMs;

        const QLocale loc;
        setStatValue(statNodes, loc.toString(qlonglong(nodes)));
        setStatValue(statTime, QStringLiteral("%1 ms").arg(timeMs, 0, 'f', 1));
        setStatValue(statTotal, loc.toString(qlonglong(totalNodes)));
        setStatValue(statTotalTime, QStringLiteral("%1 ms").arg(totalTime, 0, 'f', 1));
    });

    // Nouvelle partie : on repart de zéro (sauf la profondeur, invariante).
    connect(wPuissance4, &WPuissance4::statsReset, this, [this] {
        totalNodes = 0;
        totalTime = 0.0;
        setStatValue(statNodes, QStringLiteral("—"));
        setStatValue(statTime, QStringLiteral("—"));
        setStatValue(statTotal, QStringLiteral("—"));
        setStatValue(statTotalTime, QStringLiteral("—"));
    });
}

MainWindow::~MainWindow() {

}
