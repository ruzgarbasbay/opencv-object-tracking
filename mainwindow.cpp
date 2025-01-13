#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QVBoxLayout>
#include <opencv2/tracking.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), cap(0), cameraTimer(new QTimer(this))
{

    ui->setupUi(this);
    ui->radioButton_KCF->setChecked(true);

    if (!cap.isOpened())
    {
        QMessageBox::critical(this, "Error", "Failed to open the camera.");
        return;
    }

    connect(cameraTimer, &QTimer::timeout, this, &MainWindow::updateCameraFeed);
    cameraTimer->start(30);

    // default tracker
    tracker = cv::TrackerKCF::create();

    connect(ui->radioButton_KCF, &QRadioButton::clicked, [this](bool checked)
    {
        if (checked) trackingAlgorithm = TrackingAlgorithm::KCF;
    });
    connect(ui->radioButton_CSRT, &QRadioButton::clicked, [this](bool checked)
    {
        if (checked) trackingAlgorithm = TrackingAlgorithm::CSRT;
    });
    connect(ui->radioButton_MIL, &QRadioButton::clicked, [this](bool checked)
    {
        if (checked) trackingAlgorithm = TrackingAlgorithm::MIL;
    });
}

MainWindow::~MainWindow()
{
    cap.release();
    tracker.release();
    delete cameraTimer;
    delete ui;
}

void MainWindow::updateCameraFeed()
{
    cap >> currentFrame;
    if (currentFrame.empty())
    {
        return;
    }


    if (tracker && selectedROI.width > 0 && selectedROI.height > 0)
    {

        try
        {
            quint64 startTime = QDateTime::currentMSecsSinceEpoch();
            bool ok = tracker->update(currentFrame, selectedROI);
            if (ok)
            {
                cv::rectangle(currentFrame, selectedROI, cv::Scalar(0, 255, 0), 2);
                cv::Mat zoomedROI = currentFrame(selectedROI);
                displayImage(zoomedROI, ui->frame_zoomedImage);
            }
            quint64 endTime = QDateTime::currentMSecsSinceEpoch();
            calculateAndDisplayPerformanceMetrics(startTime, endTime);
        }

        catch (cv::Exception e)
        {
            QMessageBox::warning(this, "Warning", "Tracking failed. Please reselect ROI.");
            tracker.release();

            selectedROI = cv::Rect();
        }

    }

    displayImage(currentFrame, ui->frame_camera_feed);

}

void MainWindow::calculateAndDisplayPerformanceMetrics(quint64 startTime, quint64 endTime)
{
    short frameTime = endTime - startTime;
    float fps = 1000.0 / frameTime;

    ui->label_fps->setText("FPS: " + QString::number(qRound(fps)));
    ui->label_frametime->setText("Frametime: " + QString::number(frameTime) + "ms");
}

void MainWindow::on_button_selectROI_clicked()
{
    cv::Mat frameToSelect = currentFrame.clone();
    selectedROI = cv::selectROI("Select ROI", frameToSelect);
    cv::destroyWindow("Select ROI");

    if (selectedROI.width > 0 && selectedROI.height > 0)
    {
        initializeTracker(trackingAlgorithm);
    }
}

void MainWindow::initializeTracker(TrackingAlgorithm algorithm)
{
    if (tracker) {
        tracker.release();
    }
    switch (algorithm)
    {
    case TrackingAlgorithm::KCF:
        tracker = cv::TrackerKCF::create();
        break;
    case TrackingAlgorithm::CSRT:
        tracker = cv::TrackerCSRT::create();
        break;
    case TrackingAlgorithm::MIL:
        tracker = cv::TrackerMIL::create();
        break;
    default:
        tracker = cv::TrackerKCF::create();
        break;
    }

    tracker->init(currentFrame, selectedROI);
}

void MainWindow::displayImage(const cv::Mat &image, QFrame *frame)
{
    cv::Mat rgbImage;

    if (image.channels() == 3)
    {
        cv::cvtColor(image, rgbImage, cv::COLOR_BGR2RGB);
    }
    else
    {
        rgbImage = image.clone();
    }

    QImage qimg(rgbImage.data, rgbImage.cols, rgbImage.rows, rgbImage.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qimg).scaled(frame->size(), Qt::KeepAspectRatio);

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(frame->layout());
    if (!layout)
    {
        layout = new QVBoxLayout(frame);
        frame->setLayout(layout);
    }

    while (!layout->isEmpty())
    {
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
