/*
  Q Light Controller Plus
  rgbgrabber.cpp

  Copyright (c) Heikki Junnila
  Copyright (c) Jano Svitok
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

#include "rgbgrabber.h"
#include "qlcmacros.h"
#include "doc.h"

#define KXMLQLCRGBGrabberSource       QString("Source")
#define KXMLQLCRGBGrabberScaling      QString("Scaling")
#define KXMLQLCRGBGrabberTurning      QString("Turning")
#define KXMLQLCRGBGrabberFlipping     QString("Flipping")
#define KXMLQLCRGBGrabberOffsetX      QString("X")
#define KXMLQLCRGBGrabberOffsetY      QString("Y")

#define CAMERA 1

RGBGrabber::RGBGrabber(Doc * doc)
    : RGBAlgorithm(doc)
    , m_source("")
    , m_imageScaling(scaledXY)
    , m_xOffset(0)
    , m_yOffset(0)
    , m_imageTurning(noturn)
    , m_imageFlipping(original)
{
}

RGBGrabber::RGBGrabber(const RGBGrabber& i)
    : RGBAlgorithm(i.doc())
    , m_source(i.source())
    , m_imageScaling(i.imageScaling())
    , m_xOffset(i.m_xOffset)
    , m_yOffset(i.m_yOffset)
    , m_imageTurning(i.imageTurning())
    , m_imageFlipping(i.imageFlipping())
{
}

RGBGrabber::~RGBGrabber()
{
}

RGBAlgorithm* RGBGrabber::clone() const
{
    RGBGrabber* image = new RGBGrabber(*this);
    return static_cast<RGBAlgorithm*> (image);
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
    const QList<QScreen*> screens = QGuiApplication::screens();
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QStringList list;

    for (const QScreen* screen: screens)
    {
        QString entry = screen->name();
        entry.prepend("screen:");
        list.append(entry);
    }
#if CAMERA // camera
    for (const QCameraInfo &cameraInfo : cameras) {
        QString entry = cameraInfo.deviceName();
        entry.prepend("camera:");
        list.append(entry);
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

void RGBGrabber::rgbMapSetColors(QVector<uint> &colors)
{
    Q_UNUSED(colors);
}

QVector<uint> RGBGrabber::rgbMapGetColors()
{
    return QVector<uint>();
}

void RGBGrabber::rgbMap(const QSize& size, uint rgb, int step, RGBMap &map)
{
    Q_UNUSED(rgb);
    Q_UNUSED(step);
    QMutexLocker locker(&m_mutex);
    QImage image;

    int xOffs = xOffset();
    int yOffs = yOffset();

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
            image = screen->grabWindow(0).toImage();
    }
#if CAMERA // camera
    else if (m_source.startsWith("camera:")) {
        const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
        QCamera* camera = NULL;

        // Get the camera by name
        for (const QCameraInfo &cameraInfo : cameras) {
            QString search = cameraInfo.deviceName();
            search.prepend("camera:");
            if (search == m_source) {
                camera = new QCamera(cameraInfo);
                break;
            }
        }
        // Get the next image
        if (camera != NULL)
        {
            QCameraImageCapture* capture = new QCameraImageCapture(camera);
            capture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
            camera->setCaptureMode(QCamera::CaptureStillImage);
            camera->start();
            camera->searchAndLock();
            capture->capture();
            camera->unlock();
            // Listen for QCameraImageCapture::imageAvailable()
            //connect(capture, &QCameraImageCapture::imageCaptured, [&](int id, const QImage &preview){
                // Get the preview into the image
                QVideoFrame videoFrame;
            //});
            image = QImage(videoFrame.bits(),
                videoFrame.width(),
                videoFrame.height(),
                videoFrame.bytesPerLine(),
                QVideoFrame::imageFormatFromPixelFormat(videoFrame.pixelFormat()));
        }
    }
#endif // camera
    else
        return;
    // Check if input image size is valid (width & height > 0)
    if (image.width() == 0 || image.height() == 0)
        return;

    // Turn the image
    switch (imageTurning())
    {
        default:
        case noturn:
            break;
        case turn90:
            image = image.transformed(QMatrix().rotate(90.0));
            break;
        case turn180:
            image = image.transformed(QMatrix().rotate(180.0));
            break;
        case turn270:
            image = image.transformed(QMatrix().rotate(270.0));
            break;
    }

    // Flip the image
    switch (imageFlipping())
    {
        default:
        case original:
            break;
        case vertically:
            image = image.mirrored(false, true);
            break;
        case horizontally:
            image = image.mirrored(true, false);
            break;
    }

    // Scale the current image to target size
    if (imageScaling() == scaledWidth ||
            (imageScaling() == minWidthHeight && size.width() <= size.height()) ||
            (imageScaling() == maxWidthHeight && size.width() > size.height()))
    {
        int newHeight = ceil(image.height() * size.width(), image.width());
        // Center the image
        int scalingOffset = newHeight - size.height();
        if (scalingOffset < 0)
            scalingOffset *= -1;
        yOffs += ceil(scalingOffset, 2);
        image = image.scaled(size.width(), newHeight, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
    }
    else if (imageScaling() == scaledHeight ||
            (imageScaling() == minWidthHeight && size.width() >= size.height()) ||
            (imageScaling() == maxWidthHeight && size.width() < size.height()))
    {
        int newWidth = ceil(image.width() * size.height(), image.height());
        // Center the image
        int scalingOffset = newWidth - size.width();
        if (scalingOffset < 0)
            scalingOffset *= -1;
        xOffs += ceil(scalingOffset, 2);
        image = image.scaled(newWidth, size.height(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
    }
    else
    {
        image = image.scaled(size, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    // Prepare map and Fill the colors
    map.resize(size.height());
    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
        for (int x = 0; x < size.width(); x++)
        {
            int x1 = (x + xOffs) % image.width();
            int y1 = (y + yOffs) % image.height();

            map[y][x] = image.pixel(x1,y1);
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
            setSource(doc()->denormalizeComponentPath(root.readElementText()));
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
            QString str;
            int value;
            bool ok;
            QXmlStreamAttributes attrs = root.attributes();

            setImageScaling(stringToImageScaling(root.readElementText()));

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
            // root.skipCurrentElement();
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
    doc->writeTextElement(KXMLQLCRGBGrabberSource, this->doc()->normalizeComponentPath(m_source));

    // Scaling
    doc->writeStartElement(KXMLQLCRGBGrabberScaling);
    doc->writeAttribute(KXMLQLCRGBGrabberOffsetX, QString::number(xOffset()));
    doc->writeAttribute(KXMLQLCRGBGrabberOffsetY, QString::number(yOffset()));
    doc->writeCDATA(imageScalingToString(imageScaling()));
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
