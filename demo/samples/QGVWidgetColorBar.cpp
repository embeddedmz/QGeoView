/***************************************************************************
 * QGeoView is a Qt / C ++ widget for visualizing geographic data.
 * Copyright (C) 2018-2020 Andrey Yaroshenko.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see https://www.gnu.org/licenses.
 ****************************************************************************/

#include "QGVWidgetColorBar.h"
#include "colorMapPresets.h"

#include <QGeoView/QGVMapQGView.h>

#include <QPaintEvent>
#include <QPainter>
#include <QtMath>

namespace {
const int defaultLengthPixel = 150;
const int minLengthPixel = 130;
}

struct QGVWidgetColorBar::Internals
{
    Qt::Orientation mOrientation;

    double Min = 0.;
    double Max = 1.;
    int Width = 10;
    LinearColorMap ColorMap = ColorMapPresets::controlPointsToLinearColorMap(ColorMapPresets::Jet());
};

QGVWidgetColorBar::QGVWidgetColorBar(Qt::Orientation orientation)
    : mInternals(new QGVWidgetColorBar::Internals)
{
    mInternals->mOrientation = orientation;
    if (mInternals->mOrientation == Qt::Horizontal) {
        setAnchor(QPoint(10, 10), QSet<Qt::Edge>() << Qt::RightEdge << Qt::BottomEdge);
    } else {
        setAnchor(QPoint(10, 15 + fontMetrics().height()), QSet<Qt::Edge>() << Qt::RightEdge << Qt::BottomEdge);
    }

    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setVisible(false);
}

QGVWidgetColorBar::~QGVWidgetColorBar()
{
    delete mInternals;
}

void QGVWidgetColorBar::setOrientation(Qt::Orientation orientation)
{
    mInternals->mOrientation = orientation;
    if (getMap() == nullptr) {
        return;
    }
    const QGVCameraState camState = getMap()->getCamera();
    onCamera(camState, camState);
}

Qt::Orientation QGVWidgetColorBar::getOrientation() const
{
    return mInternals->mOrientation;
}

void QGVWidgetColorBar::paintEvent(QPaintEvent* /*event*/)
{
    // TODO : add some labels near the color bar... a title too...

    if (size().isEmpty()) {
        return;
    }

    QColor c;

    if (mInternals->mOrientation == Qt::Horizontal) {
        QRect paintRect = QRect(QPoint(0, 0), size());

        QPixmap pixmap(paintRect.size());
        pixmap.fill(Qt::transparent);

        QPainter pmPainter(&pixmap);
        //pmPainter.translate(-paintRect.x(), -paintRect.y()); // ?


        for (int x = paintRect.left(); x <= paintRect.right(); x++) {
            double value = mInternals->Min + ((x - paintRect.left()) / paintRect.width()) * (mInternals->Max - mInternals->Min);
            
            c.setRgba(mInternals->ColorMap.rgb(mInternals->Min, mInternals->Max, value));

            pmPainter.setPen(c);
            pmPainter.drawLine(x, paintRect.top(), x, paintRect.bottom());
        }
        pmPainter.end();

        // draw pixmap
        QPainter painter(this);
        painter.drawPixmap(paintRect, pixmap);

    } else {
        QRect paintRect = QRect(QPoint(0, 0), size()); // ????????? todo

        QPixmap pixmap(paintRect.size());
        pixmap.fill(Qt::transparent);

        QPainter pmPainter(&pixmap);
        // pmPainter.translate(-paintRect.x(), -paintRect.y()); // ? todo ?
        
        pmPainter.setTransform(QGV::createTransfromAzimuth(rect().center(), -90.0));

        for (int y = paintRect.top(); y <= paintRect.bottom(); y++) {
            double value = mInternals->Min +
                           ((y - paintRect.top()) / paintRect.height()) * (mInternals->Max - mInternals->Min);

            c.setRgba(mInternals->ColorMap.rgb(mInternals->Min, mInternals->Max, value));

            pmPainter.setPen(c);
            pmPainter.drawLine(paintRect.left(), y, paintRect.right(), y);
        }
        pmPainter.end();

        // draw pixmap
        QPainter painter(this);
        painter.drawPixmap(paintRect, pixmap);
    }

}

void QGVWidgetColorBar::setWidth(const int width)
{
    if (mInternals->Width != width) {
        mInternals->Width = width;
        // TODO trigger a redraw...
    }
}

int QGVWidgetColorBar::getWidth() const
{
    return mInternals->Width;
}

void QGVWidgetColorBar::setColorMap(const double min, const double max, const LinearColorMap& lcm)
{
    mInternals->ColorMap = lcm;
    mInternals->Min = min;
    mInternals->Max = max;
    // TODO trigger a redraw...
}

LinearColorMap QGVWidgetColorBar::getColorMap() const
{
    return mInternals->ColorMap;
}

double QGVWidgetColorBar::getMin() const
{
    return mInternals->Min;
}

double QGVWidgetColorBar::getMax() const
{
    return mInternals->Max;
}
