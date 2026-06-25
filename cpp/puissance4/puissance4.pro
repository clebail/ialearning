QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    puissance4.cpp \
    wpuissance4.cpp

HEADERS += \
    mainwindow.h \
    puissance4.h \
    wpuissance4.h

FORMS += \
    mainwindow.ui \
    wpuissance4.ui

RESOURCES += \
    resources.qrc

# Icône du bundle macOS (Dock / Finder) — réglée via Info.plist par qmake.
macx: ICON = icon.icns
# Icône de l'exécutable Windows.
win32: RC_ICONS = icon.ico

TRANSLATIONS += \
    puissance4_fr_FR.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
