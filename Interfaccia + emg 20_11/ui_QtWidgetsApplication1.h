/********************************************************************************
** Form generated from reading UI file 'QtWidgetsApplication1.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTWIDGETSAPPLICATION1_H
#define UI_QTWIDGETSAPPLICATION1_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtWidgetsApplication1Class
{
public:
    QWidget *centralWidget;
    QPushButton *stopRecordingButton;
    QPushButton *avviaKinect;
    QLabel *colorLabel;
    QLabel *depthLabel;
    QLabel *infraredLabel;
    QPushButton *startRecordingButton;
    QLineEdit *fileNameInput;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtWidgetsApplication1Class)
    {
        if (QtWidgetsApplication1Class->objectName().isEmpty())
            QtWidgetsApplication1Class->setObjectName("QtWidgetsApplication1Class");
        QtWidgetsApplication1Class->resize(1205, 676);
        centralWidget = new QWidget(QtWidgetsApplication1Class);
        centralWidget->setObjectName("centralWidget");
        stopRecordingButton = new QPushButton(centralWidget);
        stopRecordingButton->setObjectName("stopRecordingButton");
        stopRecordingButton->setGeometry(QRect(10, 370, 201, 91));
        QFont font;
        font.setPointSize(17);
        stopRecordingButton->setFont(font);
        avviaKinect = new QPushButton(centralWidget);
        avviaKinect->setObjectName("avviaKinect");
        avviaKinect->setGeometry(QRect(10, 130, 201, 91));
        avviaKinect->setFont(font);
        avviaKinect->setAutoDefault(false);
        colorLabel = new QLabel(centralWidget);
        colorLabel->setObjectName("colorLabel");
        colorLabel->setGeometry(QRect(290, 70, 521, 241));
        depthLabel = new QLabel(centralWidget);
        depthLabel->setObjectName("depthLabel");
        depthLabel->setGeometry(QRect(300, 320, 491, 301));
        infraredLabel = new QLabel(centralWidget);
        infraredLabel->setObjectName("infraredLabel");
        infraredLabel->setGeometry(QRect(830, 60, 521, 241));
        startRecordingButton = new QPushButton(centralWidget);
        startRecordingButton->setObjectName("startRecordingButton");
        startRecordingButton->setGeometry(QRect(10, 250, 201, 91));
        startRecordingButton->setFont(font);
        startRecordingButton->setAutoDefault(false);
        fileNameInput = new QLineEdit(centralWidget);
        fileNameInput->setObjectName("fileNameInput");
        fileNameInput->setGeometry(QRect(10, 60, 201, 31));
        QtWidgetsApplication1Class->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QtWidgetsApplication1Class);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 1205, 21));
        QtWidgetsApplication1Class->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QtWidgetsApplication1Class);
        mainToolBar->setObjectName("mainToolBar");
        QtWidgetsApplication1Class->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(QtWidgetsApplication1Class);
        statusBar->setObjectName("statusBar");
        QtWidgetsApplication1Class->setStatusBar(statusBar);

        retranslateUi(QtWidgetsApplication1Class);

        QMetaObject::connectSlotsByName(QtWidgetsApplication1Class);
    } // setupUi

    void retranslateUi(QMainWindow *QtWidgetsApplication1Class)
    {
        QtWidgetsApplication1Class->setWindowTitle(QCoreApplication::translate("QtWidgetsApplication1Class", "QtWidgetsApplication1", nullptr));
        stopRecordingButton->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "Stop Recording", nullptr));
        avviaKinect->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "Avvia Kinect", nullptr));
        colorLabel->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "colorLabel", nullptr));
        depthLabel->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "depthLabel", nullptr));
        infraredLabel->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "infraredLabel", nullptr));
        startRecordingButton->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "Start Recording", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtWidgetsApplication1Class: public Ui_QtWidgetsApplication1Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTWIDGETSAPPLICATION1_H
