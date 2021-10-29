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

#include "placemarkCluster.h"

#include <QBrush>
#include <QPainter>
#include <QPen>

struct PlacemarkCluster::Internals
{
    PlacemarkCluster::Internals(const QGV::GeoPos& geoPos, const QPointF& mapPos, const size_t count)
        : clusterGeoPos(geoPos), clusterMapPos(mapPos), count(count)
    {
    }

    const QGV::GeoPos clusterGeoPos;
    const QPointF clusterMapPos;
    const size_t count;
};

PlacemarkCluster::PlacemarkCluster(const QGV::GeoPos& geoPos, const QPointF& mapPos, const size_t count)
    : mInternals(new PlacemarkCluster::Internals(geoPos, mapPos, count))
{
    setSelectable(false); // true: plus tard

    setFlag(QGV::ItemFlag::IgnoreScale);
    setFlag(QGV::ItemFlag::IgnoreAzimuth);
}

PlacemarkCluster::~PlacemarkCluster()
{
    delete mInternals;
}

// already done by the layer and stored in mInternals->clusterMapPos
/* void PlacemarkCluster::onProjection(QGVMap* geoMap)
{
    QGVDrawItem::onProjection(geoMap);
    mInternals->clusterMapPos = geoMap->getProjection()->geoToProj(mInternals->clusterGeoPos);
}*/

QPainterPath PlacemarkCluster::projShape() const
{
    QPainterPath path;
    path.addRect(QRectF(mInternals->clusterMapPos.x() - 32,
        mInternals->clusterMapPos.y() - 32, 64, 64));
    return path;
}

void PlacemarkCluster::projPaint(QPainter* painter)
{
    QBrush greenBrush(Qt::GlobalColor::red);
    painter->setPen(
            QPen(greenBrush, 1, Qt::PenStyle::SolidLine, Qt::PenCapStyle::RoundCap, Qt::PenJoinStyle::RoundJoin));
    painter->setBrush(QBrush(greenBrush));
    painter->drawEllipse(mInternals->clusterMapPos, 32, 32);

    const QString strPoiCount = QString::number(mInternals->count);
    painter->setFont(QFont("Arial", 36, QFont::Bold));
    QFontMetrics fm(painter->font());
    const int poiCountPixelSize = fm.width(strPoiCount);
    const int poiCountPixelSizeHeight = fm.height();
    painter->setPen(Qt::white);
    painter->drawText(mInternals->clusterMapPos.x() - poiCountPixelSize / 2,
        mInternals->clusterMapPos.y() + 16, strPoiCount);
}

QString PlacemarkCluster::projTooltip(const QPointF& projPos) const
{
    auto geo = getMap()->getProjection()->projToGeo(projPos);
    return "Cluster of " + QString::number(mInternals->count) + " markers.\nPosition " +
        geo.latToString() + " " + geo.lonToString();
}

void PlacemarkCluster::projOnMouseClick(const QPointF& projPos)
{
    setOpacity(qMax(0.2, getOpacity() - 0.2));
    qInfo() << "single click on a cluster" << projPos;
}

void PlacemarkCluster::projOnMouseDoubleClick(const QPointF& projPos)
{
    setOpacity(1.0);
    qInfo() << "double click on a cluster" << projPos;
}
