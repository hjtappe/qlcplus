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

#include "rgbgrabber.h"
#include "qlcmacros.h"
#include "doc.h"

#define KXMLQLCRGBGrabberSource       QString("Source")
#define KXMLQLCRGBGrabberScaling      QString("Scaling")
#define KXMLQLCRGBGrabberTurning      QString("Turning")
#define KXMLQLCRGBGrabberFlipping     QString("Flipping")
#define KXMLQLCRGBGrabberOffsetX      QString("X")
#define KXMLQLCRGBGrabberOffsetY      QString("Y")


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
    QList<QScreen*> screens = QGuiApplication::screens();
    QStringList list;

    for (QScreen* screen: screens)
    {
        list.append(screen->name());
    }
    list.sort(Qt::CaseInsensitive);

    return list;
}

/****************************************************************************
 * Image Processing: Turning
 ****************************************************************************/

void RGBGrabber::setImageTurning(QString value)
{
    RGBGrabber::ImageTurning turningType = RGBGrabber::stringToImageTurning(value);
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
            return QString("Original");
        case turn90:
            return QString("90°");
        case turn180:
            return QString("180°");
        case turn270:
            return QString("270°");
    }
}

RGBGrabber::ImageTurning RGBGrabber::stringToImageTurning(const QString& str)
{
    if (str == QString("90°"))
        return turn90;
    else if (str == QString("180°"))
        return turn180;
    else if (str == QString("270°"))
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

void RGBGrabber::setImageFlipping(QString value)
{
    RGBGrabber::ImageFlipping flippingType = RGBGrabber::stringToImageFlipping(value);
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
            return QString("Original");
        case vertically:
            return QString("Vertically");
        case horizontally:
            return QString("Horizontally");
    }
}

 RGBGrabber::ImageFlipping RGBGrabber::stringToImageFlipping(const QString& str)
{
    if (str == QString("Vertically"))
        return vertically;
    else if (str == QString("Horizontally"))
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

void RGBGrabber::setImageScaling(QString value)
{
    RGBGrabber::ImageScaling scalingType = RGBGrabber::stringToImageScaling(value);
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
            return QString("Scaled");
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

void RGBGrabber::setXOffset(QString value)
{
    bool ok = false;
    int i = value.toInt(&ok, 10);
    if (ok)
        m_xOffset = i;
    else
    {
        m_xOffset = 0;
        qWarning() << Q_FUNC_INFO << "Invalid x offset:" << value;
    }
}

int RGBGrabber::xOffset() const
{
    return m_xOffset;
}

void RGBGrabber::setYOffset(QString value)
{
    bool ok = false;
    int i = value.toInt(&ok, 10);
    if (ok)
        m_yOffset = i;
    else
    {
        m_yOffset = 0;
        qWarning() << Q_FUNC_INFO << "Invalid y offset:" << value;
    }
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
    QImage image;

    int xOffs = xOffset();
    int yOffs = yOffset();

    // Identify the configured screen
    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen *screen = NULL;

    // Get the screen by name
    for (QScreen *checkScreen : screens)
    {
        if (checkScreen->name() == m_source)
            screen = checkScreen;
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
        // image.newHeight = ceil(image.w / size.w * size.h)
        int newHeight = ceil(image.width() * size.height(), size.width());
        // Center the image
        yOffs = (newHeight - size.height()) / 2;
        if (yOffs < 0)
            yOffs *= -1;
        image = image.scaled(size.width(), newHeight, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
    }
    else if (imageScaling() == scaledHeight||
            (imageScaling() == minWidthHeight && size.width() >= size.height()) ||
            (imageScaling() == maxWidthHeight && size.width() < size.height()))
    {
        // image.newWidth = ceil(image.h / size.h * size.w)
        int newWidth = ceil(image.height() * size.width(), size.height());
        // Center the image
        xOffs = (newWidth - size.width()) / 2;
        if (xOffs < 0)
            xOffs *= -1;
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
    return QString("Screen Grabber");
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
    return RGBAlgorithm::Image;
}

int RGBGrabber::acceptColors() const
{
    return 0;
}

//QList<RGBScriptProperty> properties()
//{
//    RGBScriptProperty sourceProperty;
//    sourceProperty.m_name = "source";
//    sourceProperty.m_displayName = KXMLQLCRGBGrabberSource;
//    sourceProperty.m_type = RGBScriptProperty::List;
//    sourceProperty.m_listValues = RGBGrabber::sourceList();
//    sourceProperty.m_readMethod = "RGBGrabber::source";
//    sourceProperty.m_writeMethod = "RGBGrabber::setSource";
//
//    RGBScriptProperty turningProperty;
//    turningProperty.m_name = "turning";
//    turningProperty.m_displayName = KXMLQLCRGBGrabberTurning;
//    turningProperty.m_type = RGBScriptProperty::List;
//    turningProperty.m_listValues = RGBGrabber::imageTurnings();
//    turningProperty.m_readMethod = "RGBGrabber::imageTurning";
//    turningProperty.m_writeMethod = "RGBGrabber::setImageTurning";
//
//    RGBScriptProperty flippingProperty;
//    flippingProperty.m_name = "flipping";
//    flippingProperty.m_displayName = KXMLQLCRGBGrabberFlipping;
//    flippingProperty.m_type = RGBScriptProperty::List;
//    flippingProperty.m_listValues = RGBGrabber::imageFlippings();
//    flippingProperty.m_readMethod = "RGBGrabber::imageFlipping";
//    flippingProperty.m_writeMethod = "RGBGrabber::setImageFlipping";
//
//    RGBScriptProperty scalingProperty;
//    scalingProperty.m_name = "scaling";
//    scalingProperty.m_displayName = KXMLQLCRGBGrabberScaling;
//    scalingProperty.m_type = RGBScriptProperty::List;
//    scalingProperty.m_listValues = RGBGrabber::imageScalings();
//    scalingProperty.m_readMethod = "RGBGrabber::imageScaling";
//    scalingProperty.m_writeMethod = "RGBGrabber::setImageScaling";
//
//    RGBScriptProperty xOffsetProperty;
//    xOffsetProperty.m_name = "xOffset";
//    xOffsetProperty.m_displayName = KXMLQLCRGBGrabberOffsetX;
//    xOffsetProperty.m_type = RGBScriptProperty::Integer;
//    xOffsetProperty.m_readMethod = "RGBGrabber::xOffset";
//    xOffsetProperty.m_writeMethod = "RGBGrabber::setXOffset";
//
//    RGBScriptProperty yOffsetProperty;
//    yOffsetProperty.m_name = "xOffset";
//    yOffsetProperty.m_displayName = KXMLQLCRGBGrabberOffsetY;
//    yOffsetProperty.m_type = RGBScriptProperty::Integer;
//    yOffsetProperty.m_readMethod = "RGBGrabber::yOffset";
//    yOffsetProperty.m_writeMethod = "RGBGrabber::setYOffset";
//
//    QList<RGBScriptProperty> properties {
//        sourceProperty,
//        turningProperty,
//        flippingProperty,
//        scalingProperty,
//        xOffsetProperty,
//        yOffsetProperty
//    };
//
//    return properties;
//}

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
        else if (root.name() == KXMLQLCRGBGrabberScaling)
        {
            QString str;
            QXmlStreamAttributes attrs = root.attributes();

            setImageScaling(root.readElementText());

            setXOffset(attrs.value(KXMLQLCRGBGrabberOffsetX).toString());
            setYOffset(attrs.value(KXMLQLCRGBGrabberOffsetY).toString());
            // root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCRGBGrabberTurning)
        {
            setImageTurning(root.readElementText());
        }
        else if (root.name() == KXMLQLCRGBGrabberFlipping)
        {
            setImageFlipping(root.readElementText());
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
