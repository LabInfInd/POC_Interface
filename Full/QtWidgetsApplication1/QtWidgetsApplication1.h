#ifndef QTWIDGETSAPPLICATION1_H
#define QTWIDGETSAPPLICATION1_H

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"

#include <k4a/k4a.hpp>
#include <k4arecord/record.hpp>

#include <opencv2/opencv.hpp>

#include <QImage>
#include <QPixmap>
#include <QTcpSocket>  // Per gestire il socket TCP
#include <QFileDialog>

#include <iostream>
#include <atomic>
#include <thread>

class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget* parent = Q_NULLPTR);
    ~QtWidgetsApplication1();

private slots:
    void on_avviaKinect_clicked();
    void on_startRecordingButton_clicked();  // Slot for Start Recording
    void on_stopRecordingButton_clicked();   // Slot for Stop Recording
    void sendCommandToServer(const QString& command);
    void on_offlineProcess_clicked();

private:
    //Puntatore alla classe generata da Qt Designer che gestisce l'interfaccia utente.
    Ui::QtWidgetsApplication1Class* ui;

    k4a::device device;

    // utilizzato per gestire il thread di cattura
    std::thread* captureThread;

    //Variabile atomica booleana che indica se il thread di cattura è attualmente in esecuzione.Questo è utile per la gestione sicura del threading
    std::atomic<bool> isCapturing;

    std::atomic<bool> isRecording;

    k4a::record recording;

    k4a_device_configuration_t config;

    QTcpSocket* tcpSocket;

    void updateUI(k4a::capture&);
};

#endif // QTWIDGETSAPPLICATION1_H