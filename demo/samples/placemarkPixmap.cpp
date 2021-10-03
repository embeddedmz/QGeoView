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

#include "placemarkPixmap.h"

#include <QPainter>

struct PlacemarkPixmap::Internals
{
    PlacemarkPixmap::Internals(const QGV::GeoPos& geoPos, const QPointF& mapPos, const QPixmap& pixmap)
        : markerGeoPos(geoPos)
        , markerMapPos(mapPos)
        , pixmap(pixmap)
    {}

    const QGV::GeoPos markerGeoPos;
    const QPointF markerMapPos;
    const QPixmap pixmap;
};

PlacemarkPixmap::PlacemarkPixmap(const QGV::GeoPos& geoPos, const QPointF& mapPos, const QPixmap& pixmap)
    : mInternals(new PlacemarkPixmap::Internals(geoPos, mapPos, pixmap))
{
    setSelectable(false); // true: plus tard
}

PlacemarkPixmap::~PlacemarkPixmap()
{
    delete mInternals;
}

QPainterPath PlacemarkPixmap::projShape() const
{
    QPainterPath path;
    path.addRect(QRectF(mInternals->markerMapPos.x() - mInternals->pixmap.width() / 2,
                        mInternals->markerMapPos.y() - mInternals->pixmap.height() / 2,
                        mInternals->pixmap.width(), mInternals->pixmap.height()));
    return path;
}

void PlacemarkPixmap::projPaint(QPainter* painter)
{
    painter->drawPixmap(mInternals->markerMapPos, mInternals->pixmap);
}

QString PlacemarkPixmap::projTooltip(const QPointF& projPos) const
{
    auto geo = getMap()->getProjection()->projToGeo(projPos);
    return "Marker position " + geo.latToString() + " " + geo.lonToString();
}

void PlacemarkPixmap::projOnMouseClick(const QPointF& projPos)
{
    setOpacity(qMax(0.2, getOpacity() - 0.2));
    qInfo() << "single click on a marker" << projPos;
}

void PlacemarkPixmap::projOnMouseDoubleClick(const QPointF& projPos)
{
    setOpacity(1.0);
    qInfo() << "double click on a marker" << projPos;
}
