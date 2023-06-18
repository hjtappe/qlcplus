/*
  Q Light Controller Plus
  rgbgrabber.cpp

  Copyright (c) Massimo Callegari
  Copyright (c) Hans-Jürgen Tappe

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

/*
 * Parameters:
 * - Type of grabber and source selection (Screen, Video Source, Window)
 * - Turning (Normal, 90°, 180°, 270°)
 * - Flipping (original, vertically, horizontally)
 * - Scaling (Scaled x & y, Scaled to width, Scaled to height)
 */

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QtMultimedia>
#include <QObject>

// FIXME: Remove
#include <QTextStream>

#include "rgbgrabber.h"
#include "qlcmacros.h"
#include "doc.h"

#define KXMLQLCRGBGrabberSource       QString("Source")
#define KXMLQLCRGBGrabberScaling      QString("Scaling")
#define KXMLQLCRGBGrabberTurning      QString("Turning")
#define KXMLQLCRGBGrabberFlipping     QString("Flipping")
#define KXMLQLCRGBGrabberOffset       QString("Offset")
#define KXMLQLCRGBGrabberOffsetX      QString("X")
#define KXMLQLCRGBGrabberOffsetY      QString("Y")

#define CAMERA 1

RGBGrabber::RGBGrabber(Doc * doc)
    : RGBAlgorithm(doc)
    , m_source("")
    , m_camera(0)
    , m_imageCapture(0)
    , m_rawImage(0)
    , m_imageTurning(noturn)
    , m_imageFlipping(original)
    , m_imageScaling(scaledXY)
    , m_xOffset(0)
    , m_yOffset(0)
{
}

RGBGrabber::RGBGrabber(const RGBGrabber& i, QObject *parent)
    : QObject(parent)
    , RGBAlgorithm(i.doc())
    , m_source(i.source())
    , m_camera(0)
    , m_imageCapture(0)
    , m_rawImage(0)
    , m_imageTurning(i.imageTurning())
    , m_imageFlipping(i.imageFlipping())
    , m_imageScaling(i.imageScaling())
    , m_xOffset(i.xOffset())
    , m_yOffset(i.yOffset())
{
}

RGBGrabber::~RGBGrabber()
{
}

RGBAlgorithm* RGBGrabber::clone() const
{
    RGBGrabber* grabber = new RGBGrabber(*this);
    return static_cast<RGBAlgorithm*> (grabber);
}

/****************************************************************************
 * Image source
 ****************************************************************************/

void RGBGrabber::setSource(const QString source)
{
    m_source = source;
}

QString RGBGrabber::source() const
{
    return m_source;
}

QStringList RGBGrabber::sourceList()
{
    QStringList list;

    const QList<QScreen*> screens = QGuiApplication::screens();
    for (const QScreen* screen: screens)
    {
        QString entry = screen->name();
        entry.prepend("screen:");
        list.append(entry);
    }

#if CAMERA // camera
    // Get the list of available cameras
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : cameras)
    {
        QCamera cam = QCamera(cameraInfo);
        if (cam.isCaptureModeSupported(QCamera::CaptureStillImage))
        {
            QString entry = cameraInfo.description(); // or deviceName()
            entry.prepend("input:");
            list.append(entry);
        }
        else
        {
            qDebug() << cameraInfo.description() << ": no image capture supported";
        }
    }
#endif // camera

    list.sort(Qt::CaseInsensitive);

    return list;
}

/****************************************************************************
 * Image Processing: Turning
 ****************************************************************************/

void RGBGrabber::setImageTurning(RGBGrabber::ImageTurning turningType)
{
    if (turningType >= noturn && turningType <= turn270)
        m_imageTurning = turningType;
    else
        m_imageTurning = noturn;
}

RGBGrabber::ImageTurning RGBGrabber::imageTurning() const
{
    return m_imageTurning;
}

QString RGBGrabber::imageTurningToString(RGBGrabber::ImageTurning turningType)
{
    switch (turningType)
    {
        default:
        case noturn:
            return QString("No turning");
        case turn90:
            return QString("Turn 90°");
        case turn180:
            return QString("Turn 180°");
        case turn270:
            return QString("Turn 270°");
    }
}

RGBGrabber::ImageTurning RGBGrabber::stringToImageTurning(const QString& str)
{
    if (str == QString("Turn 90°"))
        return turn90;
    else if (str == QString("Turn 180°"))
        return turn180;
    else if (str == QString("Turn 270°"))
        return turn270;
    else
        return noturn;
}

QStringList RGBGrabber::imageTurnings()
{
    QStringList list;
    list.append(imageTurningToString(noturn));
    list.append(imageTurningToString(turn90));
    list.append(imageTurningToString(turn180));
    list.append(imageTurningToString(turn270));
    return list;
}

/****************************************************************************
 * Image Processing: Flipping
 ****************************************************************************/

void RGBGrabber::setImageFlipping(RGBGrabber::ImageFlipping flippingType)
{
    if (flippingType >= original && flippingType <= horizontally)
        m_imageFlipping = flippingType;
    else
        m_imageFlipping = original;
}

RGBGrabber::ImageFlipping RGBGrabber::imageFlipping() const
{
    return m_imageFlipping;
}

QString RGBGrabber::imageFlippingToString(RGBGrabber::ImageFlipping flippingType)
{
    switch (flippingType)
    {
        default:
        case original:
            return QString("No flipping");
        case vertically:
            return QString("Flip Vertically");
        case horizontally:
            return QString("Flip Horizontally");
    }
}

 RGBGrabber::ImageFlipping RGBGrabber::stringToImageFlipping(const QString& str)
{
    if (str == QString("Flip Vertically"))
        return vertically;
    else if (str == QString("Flip Horizontally"))
        return horizontally;
    else
        return original;
}

QStringList RGBGrabber::imageFlippings()
{
    QStringList list;
    list.append(imageFlippingToString(original));
    list.append(imageFlippingToString(vertically));
    list.append(imageFlippingToString(horizontally));
    return list;
}

/****************************************************************************
 * Image Processing: Scaling
 ****************************************************************************/

void RGBGrabber::setImageScaling(RGBGrabber::ImageScaling scalingType)
{
    if (scalingType >= scaledXY && scalingType <= maxWidthHeight)
        m_imageScaling = scalingType;
    else
        m_imageScaling = scaledXY;
}

RGBGrabber::ImageScaling RGBGrabber::imageScaling() const
{
    return m_imageScaling;
}

QString RGBGrabber::imageScalingToString(RGBGrabber::ImageScaling scalingType)
{
    switch (scalingType)
    {
        default:
        case scaledXY:
            return QString("Scaled to size");
        case scaledWidth:
            return QString("Scaled to width");
        case scaledHeight:
            return QString("Scaled to height");
        case minWidthHeight:
            return QString("Scaled to min. w,h");
        case maxWidthHeight:
            return QString("Scaled to max. w,h");
    }
}

 RGBGrabber::ImageScaling RGBGrabber::stringToImageScaling(const QString& str)
{
    if (str == QString("Scaled to width"))
        return scaledWidth;
    else if (str == QString("Scaled to height"))
        return scaledHeight;
    else if (str == QString("Scaled to min. w,h"))
        return minWidthHeight;
    else if (str == QString("Scaled to max. w,h"))
        return maxWidthHeight;
    else
        return scaledXY;
}

QStringList RGBGrabber::imageScalings()
{
    QStringList list;
    list.append(imageScalingToString(scaledXY));
    list.append(imageScalingToString(scaledWidth));
    list.append(imageScalingToString(scaledHeight));
    list.append(imageScalingToString(minWidthHeight));
    list.append(imageScalingToString(maxWidthHeight));
    return list;
}

void RGBGrabber::setXOffset(int offset)
{
    m_xOffset = offset;
}

int RGBGrabber::xOffset() const
{
    return m_xOffset;
}

void RGBGrabber::setYOffset(int offset)
{
    m_yOffset = offset;
}

int RGBGrabber::yOffset() const
{
    return m_yOffset;
}

/****************************************************************************
 * RGBAlgorithm
 ****************************************************************************/

int RGBGrabber::rgbMapStepCount(const QSize& size)
{
    Q_UNUSED(size);
    QMutexLocker locker(&m_mutex);

    return 1;
}

void RGBGrabber::rgbMap(const QSize& size, uint rgb, int step, RGBMap &map)
{
    Q_UNUSED(rgb);
    Q_UNUSED(step);
    QMutexLocker locker(&m_mutex);

    int xOffs = xOffset();
    int yOffs = yOffset();

    // Prepare map
    map.resize(size.height());
    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
    }

    // FIXME: Remove
    QTextStream cout(stdout, QIODevice::WriteOnly);

    if (m_source.startsWith("screen:")) {
        // Identify the configured screen
        QList<QScreen*> screens = QGuiApplication::screens();
        QScreen *screen = NULL;

        // Get the screen by name
        for (QScreen *checkScreen : screens)
        {
            QString search = checkScreen->name();
            search.prepend("screen:");
            if (search == m_source) {
                screen = checkScreen;
                break;
            }
        }
        // Fallback to the primary screen
        if (screen == NULL)
        {
            screen = QGuiApplication::primaryScreen();
        }

        // Get the next image
        if (screen == NULL)
            return;
        else
        {
            QImage image = screen->grabWindow(0).toImage();
            m_rawImage = image;
        }
    }
#if CAMERA // camera
    else if (m_source.startsWith("input:"))
    {
        const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();

//        QCameraInfo thisCameraInfo;
        // Get the camera by name
//        if (m_camera != NULL)
//        {
//            cout << "Get existing camera info" << Qt::endl;
//            thisCameraInfo = QCameraInfo(m_camera);
//            if (thisCameraInfo) {
//                cout << "But failed to set the info for the existing camera" << Qt::endl;
//            } else {
//                cout << "and found: " << thisCameraInfo.description() << " at " << thisCameraInfo.deviceName() << Qt::endl;
//            }
//        }
//        else {
//            cout << "m_camera not available" << Qt::endl;
//        }
//        m_camera == NULL ||
//        if (thisCameraInfo.isNull() || thisCameraInfo.description() != m_source)
//        {
            cout << "Searching for cameras" << Qt::endl;
            for (const QCameraInfo &cameraInfo : cameras)
            {
                QString search = cameraInfo.description(); // or deviceName()
                search.prepend("input:");
                if (search == m_source)
                {
                    m_camera = new QCamera(cameraInfo);
                    cout << "Set Camera on " << cameraInfo.deviceName() << Qt::endl;
                    break;
                }
            }
//        }
//        else
//        {
//            cout << "Camera OK" << Qt::endl;
//        }
        // Get the next image
//        if (m_camera.isAvailable())
//        {
            cout << "Configure the Camera" << Qt::endl;
            // Configure the camera
            m_imageCapture = new QCameraImageCapture(m_camera);

            connect(m_imageCapture, &QCameraImageCapture::imageCaptured,
                    this, [&](int id, const QImage &preview)
                    {
                        Q_UNUSED(id);
                        // FIXME: Remove
                        QTextStream cout2(stdout, QIODevice::WriteOnly);
                        cout2 << "Copying captured image" << Qt::endl;
                        // Get the image
                        m_rawImage = preview;
                    });
            connect(m_camera, QOverload<QCamera::Error>::of(&QCamera::error), this, [&]()
                    {
                        qWarning() << "ERR: Camera: " << m_camera->errorString();
                    });
            connect(m_imageCapture, QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
                        this, [&](int id, const QCameraImageCapture::Error error, const QString &errorString)
                        {
                            Q_UNUSED(id);
                            Q_UNUSED(error);
                            qWarning() << "ERR: Capture: " << errorString;
                        });

            m_camera->setCaptureMode(QCamera::CaptureStillImage);
            m_camera->start();

// HERE
            // Get the image
            cout << "Get the next image" << Qt::endl;
            m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
            cout << "Call capture" << Qt::endl;
            m_imageCapture->capture();

//            m_camera->searchAndLock();
//            cam->unlock();
//            m_camera->unlock();

//        }
//        else
//        {
//            cout << "Camera is not available" << Qt::endl;
//        }

        cout << "Wait for capture" << Qt::endl;
        QTimer timer;
        QEventLoop loop;
        timer.setSingleShot(true);
        connect(m_imageCapture, &QCameraImageCapture::imageCaptured, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(20);
        loop.exec();
        if(timer.isActive())
            cout << "capture received with " << timer.remainingTime() << "ms left" << Qt::endl;
        else
            cout << "timeout after " << timer.interval() << Qt::endl;

        cout << "Done initializing capture" << Qt::endl;
    }
#endif // camera
    else
    {
        cout << "Invalid source selected" << Qt::endl;
        return;
    }

    // Check if input image size is valid (width & height > 0)
    if (m_rawImage.isNull() || m_rawImage.width() == 0 || m_rawImage.height() == 0)
    {
        cout << "No image. Terminating." << Qt::endl;
        return;
    }

    // Copy the captured image for transformation
    QImage matrixImage = m_rawImage;

    // Turn the image
    switch (imageTurning())
    {
        default:
        case noturn:
            break;
        case turn90:
            matrixImage = matrixImage.transformed(QTransform().rotate(90.0), Qt::FastTransformation);
            break;
        case turn180:
            matrixImage = matrixImage.transformed(QTransform().rotate(180.0), Qt::FastTransformation);
            break;
        case turn270:
            matrixImage = matrixImage.transformed(QTransform().rotate(270.0), Qt::FastTransformation);
            break;
    }

    // Flip the image
    switch (imageFlipping())
    {
        default:
        case original:
            break;
        case vertically:
            matrixImage = matrixImage.mirrored(false, true);
            break;
        case horizontally:
            matrixImage = matrixImage.mirrored(true, false);
            break;
    }

    // Scale the current image to target size
    if (imageScaling() == scaledWidth ||
            (imageScaling() == minWidthHeight && size.width() <= size.height()) ||
            (imageScaling() == maxWidthHeight && size.width() > size.height()))
    {
        int newHeight = ceil(matrixImage.height() * size.width(), matrixImage.width());
        // Scale the image to the target height, with a deviating newWidth
        QImage image = matrixImage.scaled(size.width(), newHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        // Copy the image into a image in target size
        matrixImage = image.copy(xOffs,
                yOffs + (newHeight - size.height()) / 2,
                size.width(),
                size.height());
    }
    else if (imageScaling() == scaledHeight ||
            (imageScaling() == minWidthHeight && size.width() >= size.height()) ||
            (imageScaling() == maxWidthHeight && size.width() < size.height()))
    {
        int newWidth = ceil(matrixImage.width() * size.height(), matrixImage.height());
        // Scale the image to the target height, with a deviating newWidth
        QImage image = matrixImage.scaled(newWidth, size.height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
        // Copy the image into a image in target size
        matrixImage = image.copy(xOffs + (newWidth - size.width()) / 2,
                yOffs,
                size.width(),
                size.height());
    }
    else
    {
        QImage image = matrixImage.scaled(size, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        matrixImage = image.copy(xOffs,
                        yOffs,
                        size.width(),
                        size.height());
    }

    // Fill the matrix
    for (int y = 0; y < size.height(); y++)
    {
        for (int x = 0; x < size.width(); x++)
        {
            map[y][x] = matrixImage.pixel(x, y);
            if (qAlpha(map[y][x]) == 0)
                map[y][x] = 0;
        }
    }
}

QString RGBGrabber::name() const
{
    return QString("Video Grabber");
}

QString RGBGrabber::author() const
{
    return QString("Hans-Jürgen Tappe");
}

int RGBGrabber::apiVersion() const
{
    return 1;
}

RGBAlgorithm::Type RGBGrabber::type() const
{
    return RGBAlgorithm::Grabber;
}

int RGBGrabber::acceptColors() const
{
    return 0;
}

bool RGBGrabber::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCRGBAlgorithmType).toString() != KXMLQLCRGBGrabber)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm is not RGB Grabber";
        return false;
    }

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCRGBGrabberSource)
        {
            setSource(root.readElementText());
        }
        else if (root.name() == KXMLQLCRGBGrabberTurning)
        {
            setImageTurning(stringToImageTurning(root.readElementText()));
        }
        else if (root.name() == KXMLQLCRGBGrabberFlipping)
        {
            setImageFlipping(stringToImageFlipping(root.readElementText()));
        }
        else if (root.name() == KXMLQLCRGBGrabberScaling)
        {
            setImageScaling(stringToImageScaling(root.readElementText()));
        }
        else if (root.name() == KXMLQLCRGBGrabberOffset)
        {
            QString str;
            int value;
            bool ok;
            QXmlStreamAttributes attrs = root.attributes();

            str = attrs.value(KXMLQLCRGBGrabberOffsetX).toString();
            ok = false;
            value = str.toInt(&ok);
            if (ok == true)
                setXOffset(value);
            else
                qWarning() << Q_FUNC_INFO << "Invalid X offset:" << str;

            str = attrs.value(KXMLQLCRGBGrabberOffsetY).toString();
            ok = false;
            value = str.toInt(&ok);
            if (ok == true)
                setYOffset(value);
            else
                qWarning() << Q_FUNC_INFO << "Invalid Y offset:" << str;
             root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown RGB Grabber tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool RGBGrabber::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCRGBAlgorithm);
    doc->writeAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBGrabber);

    // Source
    doc->writeTextElement(KXMLQLCRGBGrabberSource, m_source);

    // Scaling
    doc->writeTextElement(KXMLQLCRGBGrabberScaling, imageScalingToString(imageScaling()));

    // Scaling Offset
    doc->writeStartElement(KXMLQLCRGBGrabberOffset);
    doc->writeAttribute(KXMLQLCRGBGrabberOffsetX, QString::number(xOffset()));
    doc->writeAttribute(KXMLQLCRGBGrabberOffsetY, QString::number(yOffset()));
    doc->writeEndElement();

    // Turning
    doc->writeTextElement(KXMLQLCRGBGrabberTurning, imageTurningToString(imageTurning()));

    // Flipping
    doc->writeTextElement(KXMLQLCRGBGrabberFlipping, imageFlippingToString(imageFlipping()));


    /* End the <Algorithm> tag */
    doc->writeEndElement();

    return true;
}

uint RGBGrabber::ceil(uint x, uint y) const
{
    return x / y + (x % y != 0);
}
