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
-   CMake (for building the project)

## Usage

1. **Start the application.**
2. **Select the tracking algorithm** by clicking on the corresponding radio button (KCF, CSRT, MIL).
![image](https://github.com/user-attachments/assets/9eb63a90-f8ae-452b-bbbc-c1965025d467)
3. **Click the "Select ROI" button** to choose the region of interest in the video feed.
![image](https://github.com/user-attachments/assets/7927e220-9057-46d3-a9b2-bfd1380c4bbd)
4. The application will start tracking the selected ROI and display the zoomed-in view of the tracked object.
![image](https://github.com/user-attachments/assets/d48d7684-073f-4fae-a559-784c8bdfa27b)

## Acknowledgements

-   [Qt](https://www.qt.io/)
-   [OpenCV](https://opencv.org/)
