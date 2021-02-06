#include "cameraworker.h"

#include <fstream>
#include <iostream>

#include <QDebug>
#include <QApplication>

#include "unistd.h"

CameraWorker::CameraWorker()
{
    cameraRunning = true;
    qRegisterMetaType<QImage>("QImage&");
    data = new unsigned char[camera.getImageTypeSize(RASPICAM_FORMAT_RGB)];
}

CameraWorker::~CameraWorker()
{
}

void CameraWorker::doWork()
{
    // Open the camera
    if (!camera.open()) {
        qDebug() << "Error opening camera";
        cameraRunning = false;
    } else {
        cameraRunning = true;
    }

    // Wait for the camera
    sleep(3);

    // While the camera is on (the user has clicked the button), capture
    while (cameraRunning) {
        // Capture
        camera.grab();
        camera.retrieve(data, RASPICAM_FORMAT_RGB);

        // Convert the data and send to the caller to handle
        QImage image = QImage(data, camera.getWidth(), camera.getHeight(), QImage::Format_RGB888);
        emit handleImage(image);

        // Make the app process stopWork() if necessary
        qApp->processEvents();
        usleep(200);
    }
}

void CameraWorker::stopWork()
{
    // Set the flag to false
    cameraRunning = false;

    emit finished();
}
