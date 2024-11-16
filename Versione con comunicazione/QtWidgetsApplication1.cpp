#include "QtWidgetsApplication1.h"

QtWidgetsApplication1::QtWidgetsApplication1(QWidget* parent)
    : QMainWindow(parent),
    ui(new Ui::QtWidgetsApplication1Class),
    captureThread(nullptr),
    isCapturing(false),
    isRecording(false),
    tcpSocket(new QTcpSocket(this))

{
    ui->setupUi(this);

    connect(ui->avviaKinect, &QPushButton::clicked, this, &QtWidgetsApplication1::on_avviaKinect_clicked);
    connect(ui->startRecordingButton, &QPushButton::clicked, this, &QtWidgetsApplication1::on_startRecordingButton_clicked);
    connect(ui->stopRecordingButton, &QPushButton::clicked, this, &QtWidgetsApplication1::on_stopRecordingButton_clicked);

    connect(tcpSocket, &QTcpSocket::connected, this, []() {
        qDebug() << "Connessione stabilita con l'app EMG.";
        });

    connect(tcpSocket, &QTcpSocket::disconnected, this, []() {
        qDebug() << "Connessione con l'app EMG chiusa.";
        });
}


QtWidgetsApplication1::~QtWidgetsApplication1()
{
    on_stopRecordingButton_clicked();
    tcpSocket->close();
    delete ui;
}


void QtWidgetsApplication1::on_avviaKinect_clicked()
{
    if (isCapturing)
    {
        return; // Se già in acquisizione, non fare nulla
    }

    // Inizializza la configurazione del dispositivo
    config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
    config.color_resolution = K4A_COLOR_RESOLUTION_1080P;
    config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    config.camera_fps = K4A_FRAMES_PER_SECOND_30;
    config.synchronized_images_only = true;  // Sincronizzazione delle immagini

    device = k4a::device::open(K4A_DEVICE_DEFAULT);
    device.start_cameras(&config);

    isCapturing = true;;

    captureThread = new std::thread([this]() {
        while (isCapturing)
        {
            k4a::capture capture;
            if (device.get_capture(&capture, std::chrono::milliseconds(0)))
            {
                if (isRecording)
                {
                    recording.write_capture(capture);  // Scrivi il fotogramma nella registrazione
                }

                k4a::image color_image = capture.get_color_image();
                k4a::image depth_image = capture.get_depth_image();
                k4a::image ir_image = capture.get_ir_image();

                if (color_image && depth_image && ir_image)
                {
                    cv::Mat color_mat(color_image.get_height_pixels(),
                        color_image.get_width_pixels(),
                        CV_8UC4,
                        (void*)color_image.get_buffer(),
                        cv::Mat::AUTO_STEP);

                    cv::Mat depth_mat(depth_image.get_height_pixels(),
                        depth_image.get_width_pixels(),
                        CV_16U,
                        (void*)depth_image.get_buffer(),
                        cv::Mat::AUTO_STEP);

                    cv::Mat ir_mat(ir_image.get_height_pixels(),
                        ir_image.get_width_pixels(),
                        CV_16U,
                        (void*)ir_image.get_buffer(),
                        cv::Mat::AUTO_STEP);

                    cv::Mat depth_mat_8u;
                    depth_mat.convertTo(depth_mat_8u, CV_8U, 255.0 / 4096.0);

                    cv::Mat ir_mat_8u;
                    ir_mat.convertTo(ir_mat_8u, CV_8U, 255.0 / 1000.0); // Normalizza l'immagine IR

                    QImage color_qimage((uchar*)color_mat.data, color_mat.cols, color_mat.rows, color_mat.step, QImage::Format_RGB32);
                    QImage depth_qimage((uchar*)depth_mat_8u.data, depth_mat_8u.cols, depth_mat_8u.rows, depth_mat_8u.step, QImage::Format_Grayscale8);
                    QImage ir_qimage((uchar*)ir_mat_8u.data, ir_mat_8u.cols, ir_mat_8u.rows, ir_mat_8u.step, QImage::Format_Grayscale8);

                    QImage scaled_color_qimage = color_qimage.scaled(ui->colorLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    QImage scaled_depth_qimage = depth_qimage.scaled(ui->depthLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    QImage scaled_ir_qimage = ir_qimage.scaled(ui->infraredLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

                    QMetaObject::invokeMethod(this, [this, scaled_color_qimage, scaled_depth_qimage, scaled_ir_qimage]() {
                        ui->colorLabel->setPixmap(QPixmap::fromImage(scaled_color_qimage));
                        ui->depthLabel->setPixmap(QPixmap::fromImage(scaled_depth_qimage));
                        ui->infraredLabel->setPixmap(QPixmap::fromImage(scaled_ir_qimage));
                        }, Qt::QueuedConnection);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Assicurarsi di chiudere la camera fuori dal loop
        device.stop_cameras();
        device.close();
        });
}

void QtWidgetsApplication1::on_startRecordingButton_clicked()
{
    if (!isCapturing || isRecording)
    {
        return; // Ensure capturing is active and recording is not already in progress
    }

    QString filename = ui->fileNameInput->text();
    if (filename.isEmpty())
    {
        // Handle empty filename error
        ui->statusBar->showMessage("Please enter a filename for recording.");
        return;
    }

    std::string filePath = filename.toStdString() + ".mkv";

    try {
        recording = k4a::record::create(filePath.c_str(), device, config);
        recording.write_header();
        isRecording = true;
        ui->statusBar->showMessage("Recording started: " + filename);

        // Stabilisce la connessione con l'app EMG
        tcpSocket->connectToHost("127.0.0.1", 12345);  // IP e porta dell'app EMG in ascolto

        if (!tcpSocket->waitForConnected(3000)) {
            qDebug() << "Errore nella connessione all'app EMG.";
        }
        else {
            // Invia un messaggio di avvio all'app EMG
            QByteArray startMessage = "Start EMG Recording";
            tcpSocket->write(startMessage);
        }


    }
    catch (const std::exception& e) {
        ui->statusBar->showMessage("Error starting recording.");
        isRecording = false;
    }
}

void QtWidgetsApplication1::on_stopRecordingButton_clicked()
{
    // Stop recording if it is active
    if (isRecording)
    {
        try {
            recording.flush();
            recording.close();
            isRecording = false;
            ui->statusBar->showMessage("Recording stopped and saved.");

            // Invia il messaggio di arresto all'app EMG
            QByteArray stopMessage = "Stop EMG Recording";
            tcpSocket->write(stopMessage);

            // Chiudi la connessione TCP
            tcpSocket->disconnectFromHost();
        }
        catch (const std::exception& e) {
            ui->statusBar->showMessage("Error stopping recording.");
        }
    }

    // Stop image capture if it is active
    if (isCapturing)
    {
        isCapturing = false;

        if (captureThread && captureThread->joinable())
        {
            captureThread->join();
            delete captureThread;
            captureThread = nullptr;
        }

        // Clear the displayed images on the labels
        QMetaObject::invokeMethod(this, [this]() {
            ui->colorLabel->clear();
            ui->depthLabel->clear();
            ui->infraredLabel->clear();
            }, Qt::QueuedConnection);
    }
}