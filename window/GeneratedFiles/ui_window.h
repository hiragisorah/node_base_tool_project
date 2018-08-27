/********************************************************************************
** Form generated from reading UI file 'window.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_windowClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *windowClass)
    {
        if (windowClass->objectName().isEmpty())
            windowClass->setObjectName(QStringLiteral("windowClass"));
        windowClass->resize(600, 400);
        menuBar = new QMenuBar(windowClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        windowClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(windowClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        windowClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(windowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        windowClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(windowClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        windowClass->setStatusBar(statusBar);

        retranslateUi(windowClass);

        QMetaObject::connectSlotsByName(windowClass);
    } // setupUi

    void retranslateUi(QMainWindow *windowClass)
    {
        windowClass->setWindowTitle(QApplication::translate("windowClass", "window", nullptr));
    } // retranslateUi

};

namespace Ui {
    class windowClass: public Ui_windowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WINDOW_H
