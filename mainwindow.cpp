#include "./ui_mainwindow.h"
#include "mainwindow.h"
#include <QDateTime>
#include <QMessageBox>
#include <QVBoxLayout>
#include <opencv2/tracking.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), cap(0),
    cameraTimer(new QTimer(this)) {

    ui->setupUi(this);
    ui->radioButton_TLD->setHidden(true);
    ui->radioButton_MIL->setHidden(true);
    this->setFixedSize(QSize(854, 480));

    if (!cap.isOpened()) {
        QMessageBox::critical(this, "Error", "Failed to open the camera.");
        return;
    }

    connect(cameraTimer, &QTimer::timeout, this, &MainWindow::updateCameraFeed);
    cameraTimer->start(16);

    connect(ui->radioButton_KCF, &QRadioButton::clicked, [this](bool checked) {
        updateTrackingAlgorithm(TrackingAlgorithm::KCF, checked);
    });
    connect(ui->radioButton_CSRT, &QRadioButton::clicked, [this](bool checked) {
        updateTrackingAlgorithm(TrackingAlgorithm::CSRT, checked);
    });
    connect(ui->radioButton_MOSSE, &QRadioButton::clicked, [this](bool checked) {
        updateTrackingAlgorithm(TrackingAlgorithm::MOSSE, checked);
    });
    // connect(ui->radioButton_TLD, &QRadioButton::clicked, [this](bool checked) {
    //     updateTrackingAlgorithm(TrackingAlgorithm::TLD, checked);
    // });
    // connect(ui->radioButton_MIL, &QRadioButton::clicked, [this](bool checked) {
    //     updateTrackingAlgorithm(TrackingAlgorithm::MIL, checked);
    // });
}

MainWindow::~MainWindow() {
    cap.release();
    tracker.release();
    delete cameraTimer;
    delete ui;
}

void MainWindow::updateTrackingAlgorithm(TrackingAlgorithm algo, bool checked) {
    if (checked && trackingAlgorithm != algo) {
        trackingAlgorithm = algo;
        initializeTracker(algo);
    }
}

void MainWindow::updateCameraFeed() {
    cap >> currentFrame;
    if (currentFrame.empty()) {
        return;
    }

    if (tracker && !selectedROI.empty()) {

        try {
            quint64 startTime = QDateTime::currentMSecsSinceEpoch();
            bool ok = tracker->update(currentFrame, selectedROI);
            quint64 endTime = QDateTime::currentMSecsSinceEpoch();

            if (ok) {
                cv::rectangle(currentFrame, selectedROI, cv::Scalar(0, 255, 0), 2);
                cv::Mat zoomedROI = currentFrame(selectedROI);
                displayImage(zoomedROI, ui->frame_zoomedImage);
            }

            calculatePerformanceMetrics(startTime, endTime);
        }

        catch (cv::Exception e) {
            QMessageBox::warning(this, "Warning",
                                 "Tracking failed. Please reselect ROI.");
            tracker.release();
            displayPerformanceMetrics("", "");
            selectedROI = cv::Rect();
        }
    }

    displayImage(currentFrame, ui->frame_camera_feed);
}

void MainWindow::calculatePerformanceMetrics(quint64 startTime,
                                             quint64 endTime) {
    short frameTime = endTime - startTime;

    if (frameTime == 0) {
        displayPerformanceMetrics("FPS: >1000", "Frametime: <1ms");
    } else {
        displayPerformanceMetrics(
            "FPS: " + QString::number(1000.0 / frameTime, 'f', 2),
            "Frametime: " + QString::number(frameTime) + "ms");
    }
}

void MainWindow::displayPerformanceMetrics(QString fpsLabelText,
                                           QString frametimeLabelText) {
    ui->label_fps->setText(fpsLabelText);
    ui->label_frametime->setText(frametimeLabelText);
}

void MainWindow::on_button_selectROI_clicked() {
    cv::Mat frameToSelect = currentFrame.clone();
    selectedROI = cv::selectROI("Select ROI", frameToSelect);
    cv::destroyWindow("Select ROI");

    if (trackingAlgorithm == TrackingAlgorithm::DEFAULT) {
        trackingAlgorithm = TrackingAlgorithm::KCF;
        ui->radioButton_KCF->setChecked(true);
    }

    return initializeTracker(trackingAlgorithm);
}

void MainWindow::initializeTracker(TrackingAlgorithm algorithm) {
    if (selectedROI.empty()) {
        return;
    }

    if (tracker) {
        tracker.release();
    }

    switch (algorithm) {
    case TrackingAlgorithm::KCF:
        tracker = cv::TrackerKCF::create();
        break;
    case TrackingAlgorithm::CSRT:
        tracker = cv::TrackerCSRT::create();
        break;
    case TrackingAlgorithm::MOSSE:
        tracker = cv::legacy::upgradeTrackingAPI(cv::legacy::TrackerMOSSE::create());
        break;
    // case TrackingAlgorithm::TLD:
    //     tracker = cv::legacy::upgradeTrackingAPI(cv::legacy::TrackerTLD::create());
    //     break;
    // case TrackingAlgorithm::MIL:
    //     tracker = cv::TrackerMIL::create();
    //     break;
    default:
        tracker = cv::TrackerKCF::create();
        break;
    }

    try {
        tracker->init(currentFrame, selectedROI);
    }

    catch (cv::Exception e) {
        QMessageBox::warning(this, "Warning",
            "Tracker initialization failed. Please select ROI.");
        tracker.release();
        displayPerformanceMetrics("", "");
        selectedROI = cv::Rect();
    }
}

void MainWindow::displayImage(const cv::Mat &image, QFrame *frame) {
    cv::Mat rgbImage;

    if (image.channels() == 3) {
        cv::cvtColor(image, rgbImage, cv::COLOR_BGR2RGB);
    } else {
        rgbImage = image.clone();
    }

    QImage qimg(rgbImage.data, rgbImage.cols, rgbImage.rows, rgbImage.step,
                QImage::Format_RGB888);
    QPixmap pixmap =
        QPixmap::fromImage(qimg).scaled(frame->size(), Qt::KeepAspectRatio);

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(frame->layout());
    if (!layout) {
        layout = new QVBoxLayout(frame);
        frame->setLayout(layout);
    }

    while (!layout->isEmpty()) {
        QLayoutItem *item = layout->takeAt(0);
        delete item->widget();
        delete item;
    }

    QLabel *label = new QLabel(frame);
    label->setPixmap(pixmap);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    connect(cameraTimer, &QTimer::timeout, label, &QLabel::deleteLater);
}
