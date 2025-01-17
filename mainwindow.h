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
        DEFAULT,
        KCF,
        CSRT,
        MOSSE,
        // TLD,
        // MIL,
    };

    Ui::MainWindow *m_ui;
    cv::VideoCapture m_cap;
    QTimer *m_cameraTimer;
    cv::Rect m_selectedROI;
    cv::Mat m_trackedFrame;
    cv::Mat m_currentFrame;
    TrackingAlgorithm m_trackingAlgorithm = TrackingAlgorithm::DEFAULT;
    cv::Ptr<cv::Tracker> m_tracker = nullptr;

    void displayImage(const cv::Mat &image, QFrame *frame);
    void initializeTracker(TrackingAlgorithm algorithm);
    void calculatePerformanceMetrics(quint64 startTime, quint64 endTime);
    void displayPerformanceMetrics(QString fpsLabelText,
                                   QString frametimeLabelText);
    void updateTrackingAlgorithm(TrackingAlgorithm algo, bool checked);
};

#endif // MAINWINDOW_H
