#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFrame>
#include <QImage>
#include <QMainWindow>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/tracking/tracking_legacy.hpp>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_button_selectROI_clicked();
    void updateCameraFeed();

private:
    enum class TrackingAlgorithm {
        KCF,
        CSRT,
        MOSSE,
        TLD,
        MIL,
    };

    Ui::MainWindow *ui;
    cv::VideoCapture cap;
    QTimer *cameraTimer;
    cv::Rect selectedROI;
    cv::Mat trackedFrame;
    cv::Mat currentFrame;
    TrackingAlgorithm trackingAlgorithm = TrackingAlgorithm::KCF;
    cv::Ptr<cv::Tracker> tracker = nullptr;

    void displayImage(const cv::Mat &image, QFrame *frame);
    void initializeTracker(TrackingAlgorithm algorithm);
    void calculateAndDisplayPerformanceMetrics(quint64 startTime,
                                               quint64 endTime);
};

#endif // MAINWINDOW_H
