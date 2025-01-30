/*
  Q Light Controller Plus
  rgbgraber.h

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

#ifndef RGBGRABBER_H
#define RGBGRABBER_H

#include <QMutexLocker>
#include <QString>
#include <QMovie>
#include <QImage>

#include "rgbalgorithm.h"

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCRGBGrabber "Screen Grabber"

class RGBGrabber : public RGBAlgorithm
{
public:
    RGBGrabber(Doc * doc);
    RGBGrabber(const RGBGrabber& t);
    ~RGBGrabber();

    /** @reimp */
    RGBAlgorithm* clone() const;

    /************************************************************************
     * Image source
     ************************************************************************/
public:
    /** Set source of the image */
    void setSource(const QString source);

    /** Get source of the image */
    QString source() const;

    /** List of available sources to grab from */
    static QStringList sourceList();

private:
    QString m_source;
    QMutex m_mutex;

    /************************************************************************
     * Image Processing
     ************************************************************************/
public:
    enum ImageTurning {
        noturn,
        turn90,
        turn180,
        turn270
    };

    void setImageTurning(ImageTurning turningType);
    ImageTurning imageTurning() const;

    static QString imageTurningToString(ImageTurning turningType);
    static ImageTurning stringToImageTurning(const QString& str);
    static QStringList imageTurnings();

    enum ImageFlipping {
        original,
        vertically,
        horizontally
    };

    void setImageFlipping(ImageFlipping flippingType);
    ImageFlipping imageFlipping() const;

    static QString imageFlippingToString(ImageFlipping flippingType);
    static ImageFlipping stringToImageFlipping(const QString& str);
    static QStringList imageFlippings();

    enum ImageScaling {
        scaledXY,
        scaledWidth,
        scaledHeight,
        minWidthHeight,
        maxWidthHeight
    };

    void setImageScaling(ImageScaling scalingType);
    ImageScaling imageScaling() const;
    static QString imageScalingToString(ImageScaling scalingType);
    static ImageScaling stringToImageScaling(const QString& str);
    static QStringList imageScalings();

    void setXOffset(int offset);
    int xOffset() const;

    void setYOffset(int offset);
    int yOffset() const;

private:
    ImageScaling m_imageScaling;
    int m_xOffset;
    int m_yOffset;
    ImageTurning m_imageTurning;
    ImageFlipping m_imageFlipping;


    /************************************************************************
     * RGBAlgorithm
     ************************************************************************/
public:
    /** @reimp */
    int rgbMapStepCount(const QSize& size);

    /** @reimp */
    void rgbMapSetColors(QVector<uint> &colors);

    /** @reimp */
    QVector<uint> rgbMapGetColors();

    /** @reimp */
    void rgbMap(const QSize& size, uint rgb, int step, RGBMap &map);

    /** @reimp */
    QString name() const;

    /** @reimp */
    QString author() const;

    /** @reimp */
    int apiVersion() const;

    /** @reimp */
    RGBAlgorithm::Type type() const;

    /** @reimp */
    int acceptColors() const;

    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc) const;

private:
    /**
     * Ceil() function for unsigned integer: ceil(x / y)
     * @param x
     * @param y
     * @return ceil(x / y)
     */
    uint ceil(uint x, uint y) const;
};

/** @} */

#endif
