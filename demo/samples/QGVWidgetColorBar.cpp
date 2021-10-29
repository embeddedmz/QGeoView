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
    bool mAutoAdjust;
    int mScaleMeters;
    int mScalePixels;

    double Min = 0.;
    double Max = 1.;
    int Width = 10;
    LinearColorMap ColorMap;
};

QGVWidgetColorBar::QGVWidgetColorBar(Qt::Orientation orientation)
    : mInternals(new QGVWidgetColorBar::Internals)
{
    mInternals->mAutoAdjust = true;
    mInternals->mScaleMeters = 0;
    mInternals->mScalePixels = 0;
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

void QGVWidgetColorBar::setAutoAdjust(bool autoAdjust)
{
    mInternals->mAutoAdjust = autoAdjust;
    if (getMap() == nullptr) {
        setVisible(false);
        return;
    }
    const QGVCameraState camState = getMap()->getCamera();
    onCamera(camState, camState);
}

bool QGVWidgetColorBar::getAutoAdjust() const
{
    return mInternals->mAutoAdjust;
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

QString QGVWidgetColorBar::getDistanceLabel(int meters, int accuracy) const
{
    if (meters > 1000) {
        return tr("%1 km").arg(QString::number(static_cast<double>(meters) / 1000, 'f', accuracy));
    } else {
        return tr("%1 m").arg(QString::number(static_cast<double>(meters), 'f', accuracy));
    }
}

void QGVWidgetColorBar::onCamera(const QGVCameraState& oldState, const QGVCameraState& newState)
{
    QGVWidget::onCamera(oldState, newState);
    const QPoint viewPoint1 = geometry().topLeft();
    QPoint viewPoint2;
    if (mInternals->mOrientation == Qt::Horizontal) {
        viewPoint2 = QPoint(viewPoint1.x() + defaultLengthPixel, viewPoint1.y());
    } else {
        viewPoint2 = QPoint(viewPoint1.x(), viewPoint1.y() + defaultLengthPixel);
    }

    const QPointF projPoint1 = getMap()->mapToProj(viewPoint1);
    const QPointF projPoint2 = getMap()->mapToProj(viewPoint2);
    const QGVProjection* projection = getMap()->getProjection();
    if (!projection->boundaryProjRect().contains(projPoint1) || !projection->boundaryProjRect().contains(projPoint2)) {
        resize(QSize(0, 0));
        repaint();
        return;
    }

    int newLengthPixels = defaultLengthPixel;
    int newLengthMeters = static_cast<int>(projection->geodesicMeters(projPoint1, projPoint2));
    if (mInternals->mAutoAdjust) {
        const double metersLog = qMax(1.0, log10(newLengthMeters));
        const int meters10 = static_cast<int>(qPow(10, qFloor(metersLog)));
        const double correction = static_cast<double>(meters10) / newLengthMeters;
        newLengthMeters = meters10;
        newLengthPixels = qCeil(correction * newLengthPixels);
        if (newLengthPixels < minLengthPixel) {
            const double factor = qPow(2, qCeil(log(minLengthPixel / newLengthPixels) * M_LOG2E));
            newLengthMeters *= factor;
            newLengthPixels *= factor;
        }
    }

    if (mInternals->mScaleMeters != newLengthMeters || mInternals->mScalePixels != newLengthPixels) {
        const int height = fontMetrics().boundingRect("W").height() + 5;
        mInternals->mScaleMeters = newLengthMeters;
        mInternals->mScalePixels = newLengthPixels;
        if (mInternals->mOrientation == Qt::Horizontal) {
            resize(QSize(mInternals->mScalePixels, height));
        } else {
            resize(QSize(height, mInternals->mScalePixels));
        }
        repaint();
    }
}

void QGVWidgetColorBar::paintEvent(QPaintEvent* /*event*/)
{
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

        for (int y = paintRect.top(); y <= paintRect.bottom(); y++) {
            double value = mInternals->Min +
                           ((y - paintRect.top()) / paintRect.height()) * (mInternals->Max - mInternals->Min);

            c.setRgba(mInternals->ColorMap.rgb(mInternals->Min, mInternals->Max, value));

            pmPainter.setPen(c);
            pmPainter.drawLine(paintRect.left(), y, paintRect.right(), y);
        }
        pmPainter.end();
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
