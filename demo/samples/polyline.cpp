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

#include "polyline.h"

#include <QBrush>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPen>

Polyline::Polyline(const QVector<QGV::GeoPos>& linePts, QColor color)
    : mGeoPosList(linePts)
    , mColor(color)
{
    setSelectable(false);
}

Polyline::Polyline(const QString& geoJsonStr)
{
    // todo : add a try/catch block ?

    QJsonDocument jsdoc = QJsonDocument::fromJson(geoJsonStr.toUtf8());
    QJsonArray features = jsdoc.object()["features"].toArray();
    QJsonArray::const_iterator it = features.constBegin();
    while (it != features.constEnd())
    {
        QJsonObject feature = it->toObject();
        QJsonObject geometry = feature["geometry"].toObject();
        QString geometryType = geometry["type"].toString().toUpper();
        if (geometryType == QStringLiteral("POINT"))
        {
            QJsonArray coordinates = geometry["coordinates"].toArray();
            mPointsGeoPosList.push_back(
                        QGV::GeoPos(coordinates.at(1).toDouble(),
                                    coordinates.at(0).toDouble()));
        }
        else if (geometryType == QStringLiteral("LINESTRING"))
        {
            QVector<QGV::GeoPos> linesPointsGeoPosList;

            QJsonArray coordinates = geometry["coordinates"].toArray();
            QJsonArray::const_iterator it = coordinates.constBegin();
            while (it != coordinates.constEnd())
            {
                QJsonArray lineCoords = it->toArray();
                linesPointsGeoPosList.push_back(
                            QGV::GeoPos(lineCoords.at(1).toDouble(),
                                        lineCoords.at(0).toDouble()));
                ++it;
            }
            mLinesGeoPosList.push_back(linesPointsGeoPosList);
        }

        ++it;
    }
}

void Polyline::onProjection(QGVMap* geoMap)
{
    QGVDrawItem::onProjection(geoMap);

    if (!mGeoPosList.isEmpty())
    {
        mProjLinePoints.resize(mGeoPosList.size());
        for (int i = 0; i < mGeoPosList.size(); ++i)
        {
            mProjLinePoints[i] = geoMap->getProjection()->geoToProj(mGeoPosList[i]);
        }
    }

    if (!mPointsGeoPosList.isEmpty())
    {
        mProjPoints.resize(mPointsGeoPosList.size());
        for (int i = 0; i < mPointsGeoPosList.size(); ++i)
        {
            mProjPoints[i] = geoMap->getProjection()->geoToProj(mPointsGeoPosList[i]);
        }
    }

    if (!mLinesGeoPosList.isEmpty())
    {
        mProjLines.resize(mLinesGeoPosList.size());
        for (int i = 0; i < mProjLines.size(); ++i)
        {
            mProjLines[i].resize(mLinesGeoPosList.size());
            for (int j = 0; j < mProjLines[i].size(); ++j)
            {
                mProjLines[i][j] = geoMap->getProjection()->geoToProj(
                            mLinesGeoPosList[i][j]);
            }
        }
    }
}

QPainterPath Polyline::projShape() const
{
    QPainterPath path;
    path.addPolygon(mProjLinePoints);
    //path.addPolygon(mProjPoints);
    return path;
}

void Polyline::projPaint(QPainter* painter)
{
    painter->setPen(QPen(QBrush(mColor), 1,
                         Qt::PenStyle::SolidLine,
                         Qt::PenCapStyle::RoundCap,
                         Qt::PenJoinStyle::RoundJoin));

    // draw lines
    if (!mProjLinePoints.isEmpty())
    {
        painter->drawPolyline(mProjLinePoints);
    }

    // draw geojson points
    if (!mProjPoints.isEmpty())
    {
        painter->drawPoints(mProjPoints);
    }

    // draw geojson lines
    for (const auto& linePts : mProjLines)
    {
        if (!linePts.isEmpty())
        {
            painter->drawLines(linePts);
        }
    }
}

QString Polyline::projTooltip(const QPointF& projPos) const
{
    auto geo = getMap()->getProjection()->projToGeo(projPos);
    return "Polyline with color " + mColor.name() + "\nPosition " + geo.latToString() + " " + geo.lonToString();
}
