// OFFIS Automation Framework
// Copyright (C) 2013-2017 OFFIS e.V.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "SimpleCameraQt.h"

#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QMutexLocker>

#include <opencv2/imgproc.hpp>

using namespace cv;
REGISTER_FILTER(SimpleCameraQt);

SimpleCameraQt::SimpleCameraQt() : mCamera(0), mCameraFrameGrabber(0)
{
    setName("SimpleCameraQt");
    setDesc(QObject::tr("Outputs data from a camera using QCamera"));
    setGroup("input");

    mCameraName.setName("camera");
    mCameraName.setDesc(QObject::tr("Selected camera for output"));

    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras) {
        mCameraName.addChoice(cameraInfo.description());
    }
    mCameraName.setDefault(QCameraInfo::defaultCamera().description());
    addInputPort(mCameraName);

    mOut.setName("imageOut");
    mOut.setDesc(QObject::tr("Image output"));
    addOutputPort(mOut);
}

SimpleCameraQt::~SimpleCameraQt()
{
    delete mCamera;
    delete mCameraFrameGrabber;
}

void SimpleCameraQt::initialize()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras) {
        if(cameraInfo.description() == mCameraName){
            mCameraFrameGrabber = new CameraFrameGrabber();
            connect(mCameraFrameGrabber, SIGNAL(frameAvailable(QImage)), this, SLOT(handleFrame(QImage)));

            mCamera = new QCamera(cameraInfo);
            mCamera->setCaptureMode(QCamera::CaptureVideo);
            mCamera->setViewfinder(mCameraFrameGrabber);
            mCamera->start();

            break;
        }
    }
}

void SimpleCameraQt::execute()
{  
    // Check if user might has changed the cameraName
    if(QCameraInfo(*mCamera).description() != mCameraName){
        mCamera->stop();
        initialize();
    }

    QMutexLocker lock(&mImageMutex);
    if(mCurrentImage.isNull()){
       mOut.send(Mat());
    }

    Mat mat;
    switch (mCurrentImage.format()) {
    case QImage::Format_RGB888:{
       mat = Mat(mCurrentImage.height(), mCurrentImage.width(), CV_8UC3, mCurrentImage.bits(), mCurrentImage.bytesPerLine());

    }
    case QImage::Format_Indexed8:{
       mat = Mat(mCurrentImage.height(), mCurrentImage.width(), CV_8U, mCurrentImage.bits(), mCurrentImage.bytesPerLine());
    }
    case QImage::Format_RGB32:
        mCurrentImage = mCurrentImage.convertToFormat(QImage::Format_ARGB32);
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:{
        mat = Mat(mCurrentImage.height(), mCurrentImage.width(), CV_8UC4, mCurrentImage.bits(), mCurrentImage.bytesPerLine());
    }
    default:
        break;
    }
    mOut.send(mat.clone());
}

void SimpleCameraQt::deinitialize()
{
    mCamera->stop();
}

void SimpleCameraQt::handleFrame(QImage frame)
{
    QMutexLocker lock(&mImageMutex);
    mCurrentImage = frame.copy();
}
