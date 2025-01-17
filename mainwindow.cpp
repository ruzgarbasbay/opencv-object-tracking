#include "./ui_mainwindow.h"
#include "mainwindow.h"
#include <QDateTime>
#include <QMessageBox>
#include <QVBoxLayout>
#include <opencv2/tracking.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow), m_cap(0),
    m_cameraTimer(new QTimer(this)) {

    m_ui->setupUi(this);
    m_ui->radioButton_TLD->setHidden(true);
    m_ui->radioButton_MIL->setHidden(true);
    this->setFixedSize(QSize(854, 480));

    if (!m_cap.isOpened()) {
        QMessageBox::critical(this, "Error", "Failed to open the camera.");
        return;
    }

    connect(m_cameraTimer, &QTimer::timeout, this, &MainWindow::updateCameraFeed);
    m_cameraTimer->start(16);

    connect(m_ui->radioButton_KCF, &QRadioButton::clicked, [this](bool checked) {
        updateTrackingAlgorithm(TrackingAlgorithm::KCF, checked);
    });
    connect(m_ui->radioButton_CSRT, &QRadioButton::clicked, [this](bool checked) {
        updateTrackingAlgorithm(TrackingAlgorithm::CSRT, checked);
    });
    connect(m_ui->radioButton_MOSSE, &QRadioButton::clicked, [this](bool checked) {
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
    m_cap.release();
    m_tracker.release();
    delete m_cameraTimer;
    delete m_ui;
}

void MainWindow::updateTrackingAlgorithm(TrackingAlgorithm algo, bool checked) {
    if (checked && m_trackingAlgorithm != algo) {
        m_trackingAlgorithm = algo;
        initializeTracker(algo);
    }
}

void MainWindow::updateCameraFeed() {
    m_cap >> m_currentFrame;
    if (m_currentFrame.empty()) {
        return;
    }

    if (m_tracker && !m_selectedROI.empty()) {

        try {
            quint64 startTime = QDateTime::currentMSecsSinceEpoch();
            bool ok = m_tracker->update(m_currentFrame, m_selectedROI);
            quint64 endTime = QDateTime::currentMSecsSinceEpoch();

            if (ok) {
                cv::rectangle(m_currentFrame, m_selectedROI, cv::Scalar(0, 255, 0), 2);
                cv::Mat zoomedROI = m_currentFrame(m_selectedROI);
                displayImage(zoomedROI, m_ui->frame_zoomedImage);
            }

            calculatePerformanceMetrics(startTime, endTime);
        }

        catch (cv::Exception e) {
            QMessageBox::warning(this, "Warning",
                                 "Tracking failed. Please reselect ROI.");
            m_tracker.release();
            displayPerformanceMetrics("", "");
            m_selectedROI = cv::Rect();
        }
    }

    displayImage(m_currentFrame, m_ui->frame_camera_feed);
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
    m_ui->label_fps->setText(fpsLabelText);
    m_ui->label_frametime->setText(frametimeLabelText);
}

void MainWindow::on_button_selectROI_clicked() {
    cv::Mat frameToSelect = m_currentFrame.clone();
    m_selectedROI = cv::selectROI("Select ROI", frameToSelect);
    cv::destroyWindow("Select ROI");

    if (m_trackingAlgorithm == TrackingAlgorithm::DEFAULT) {
        m_trackingAlgorithm = TrackingAlgorithm::KCF;
        m_ui->radioButton_KCF->setChecked(true);
    }

    return initializeTracker(m_trackingAlgorithm);
}

void MainWindow::initializeTracker(TrackingAlgorithm algorithm) {
    if (m_selectedROI.empty()) {
        return;
    }

    if (m_tracker) {
        m_tracker.release();
    }

    switch (algorithm) {
    case TrackingAlgorithm::KCF:
        m_tracker = cv::TrackerKCF::create();
        break;
    case TrackingAlgorithm::CSRT:
        m_tracker = cv::TrackerCSRT::create();
        break;
    case TrackingAlgorithm::MOSSE:
        m_tracker = cv::legacy::upgradeTrackingAPI(cv::legacy::TrackerMOSSE::create());
        break;
    // case TrackingAlgorithm::TLD:
    //     tracker = cv::legacy::upgradeTrackingAPI(cv::legacy::TrackerTLD::create());
    //     break;
    // case TrackingAlgorithm::MIL:
    //     tracker = cv::TrackerMIL::create();
    //     break;
    default:
        m_tracker = cv::TrackerKCF::create();
        break;
    }

    try {
        m_tracker->init(m_currentFrame, m_selectedROI);
    }

    catch (cv::Exception e) {
        QMessageBox::warning(this, "Warning",
            "Tracker initialization failed. Please select ROI.");
        m_tracker.release();
        displayPerformanceMetrics("", "");
        m_selectedROI = cv::Rect();
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

    connect(m_cameraTimer, &QTimer::timeout, label, &QLabel::deleteLater);
}
