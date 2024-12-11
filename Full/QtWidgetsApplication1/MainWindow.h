#ifndef QTMAINWINDOW_H
#define QTMAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

#include <k4a/k4a.hpp>
#include <k4abt.h>
#include <k4arecord/record.hpp>

#include <opencv2/opencv.hpp>

#include <QImage>
#include <QPixmap>
#include <QTcpSocket>  // Per gestire il socket TCP
#include <QFileDialog>
#include <QThread>
#include <ProcessWorker.h>

#include <iostream>
#include <atomic>
#include <thread>

#include "VideoProcessor.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow();

private slots:
    void on_avviaKinect_clicked();
    void on_startRecordingButton_clicked();  // Slot for Start Recording
    void on_stopRecordingButton_clicked();   // Slot for Stop Recording
    void sendCommandToServer(const QString& command);
    void on_offlineProcess_clicked();
    void handleResult(bool);

private:
    //Puntatore alla classe generata da Qt Designer che gestisce l'interfaccia utente.
    Ui::MainWindowClass* ui;

    // utilizzato per gestire il thread di cattura
    std::thread* captureThread;

    QThread workerThread;

    //Variabile atomica booleana che indica se il thread di cattura � attualmente in esecuzione.Questo � utile per la gestione sicura del threading
    std::atomic<bool> isCapturing;

    std::atomic<bool> isRecording;

    k4a::device device;

    k4a::record recording;

    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;

    k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;

    QTcpSocket* tcpSocket;

    void updateUI(k4a::capture&);

signals:
    void operate(const char* , const char* , k4abt_tracker_configuration_t& );
};

#endif // QTWIDGETSAPPLICATION1_H