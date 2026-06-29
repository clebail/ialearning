#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QLocale>
#include "mainwindow.h"
#include "transpositiontable.h"

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

    // Les réglages de l'UI pilotent directement le moteur (membres statiques partagés) ;
    // on synchronise une fois au démarrage avec l'état initial des widgets.
    Puissance4::maxDepth = depthSpin->value();
    Puissance4::evalZero = evalZeroCheck->isChecked();
    Puissance4::ttEnabled = ttCheck->isChecked();

    // Relance une partie neuve : on vide la table de transposition (ses scores en cache
    // dépendent des réglages courants → invalides dès qu'un paramètre change), on remet
    // le plateau à zéro, puis si « L'IA » commence on lui laisse poser le premier pion.
    auto restartGame = [this] {
        TranspositionTable::getInstance()->clear();
        wPuissance4->reset();
        if (radioIA->isChecked())
            wPuissance4->aiStart();
    };

    connect(rejouerButton, &QPushButton::clicked, this, restartGame);

    // Changer un réglage de recherche relance la partie de zéro : on ne mélange jamais,
    // dans une même partie, des coups calculés avec des paramètres différents.
    connect(depthSpin, qOverload<int>(&QSpinBox::valueChanged), this,
            [this, restartGame](int value) {
        Puissance4::maxDepth = value;
        restartGame();
    });
    connect(evalZeroCheck, &QCheckBox::toggled, this, [this, restartGame](bool checked) {
        Puissance4::evalZero = checked;
        restartGame();
    });
    connect(ttCheck, &QCheckBox::toggled, this, [this, restartGame](bool checked) {
        Puissance4::ttEnabled = checked;
        restartGame();
    });

    // « Qui commence » : toggled part en double (l'un se décoche, l'autre se coche) ;
    // on ne déclenche que sur le bouton qui PASSE à coché → une seule relance.
    connect(radioHuman, &QRadioButton::toggled, this, [restartGame](bool checked) {
        if (checked) restartGame();
    });
    connect(radioIA, &QRadioButton::toggled, this, [restartGame](bool checked) {
        if (checked) restartGame();
    });

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
