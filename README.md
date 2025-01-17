# OpenCV Tracking Application

This project is a Qt-based application that uses OpenCV to capture video from a camera and perform object tracking using different tracking algorithms (KCF, CSRT, MIL).

## Features

-   Capture video feed from a camera.
-   Select a Region of Interest (ROI) for tracking.
-   Switch between different tracking algorithms (KCF, CSRT, MIL).
-   Display the tracked object and zoomed-in view of the ROI.

## Requirements

-   Qt 6.x
-   OpenCV 4.x with contrib
-   CMake >= 3.16

## Usage

1. **Start the application.**
2. **Select the tracking algorithm** by clicking on the corresponding radio button (KCF, CSRT, MIL).
![image](https://github.com/user-attachments/assets/2a24d2d6-1f93-438d-8fa7-ed2c0e8828f4)

3. **Click the "Select ROI" button** to choose the region of interest in the video feed.
![image](https://github.com/user-attachments/assets/0d4b19a4-c7bb-4da1-b273-d66fe94ffea6)

4. The application will start tracking the selected ROI and display the zoomed-in view of the tracked object.
![image](https://github.com/user-attachments/assets/b72327d9-f556-4f50-b89f-b106fd0744db)


## Acknowledgements

-   [Qt](https://www.qt.io/)
-   [OpenCV](https://opencv.org/)
