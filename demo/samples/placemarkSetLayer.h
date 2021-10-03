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

#pragma once

#include <QGeoView/QGVDrawItem.h>

#include <QPointer>

class PlacemarkSetLayer : public QGVDrawItem
{
    Q_OBJECT

public:
    explicit PlacemarkSetLayer();
    ~PlacemarkSetLayer() override;

    void setImage(const QPixmap& img); // default : create a shape with QPainter
    void setClustering(const bool enable); // default false
    void setClusteringTreeDepth(size_t depth);      // default 14
    void setClusterDistance(const size_t distance);  // default 40 pixels
    
    void recomputeClusters(); // e.g. updateClusters
    
    size_t getNumberOfMarkers() const;

    size_t add(const QGV::GeoPos& pos);
    bool remove(const size_t poiId);
    void removeAll();

    bool setVisibility(const size_t poiId, const bool visible);
    bool getVisibility(const size_t poiId) const;

    /* debug */
    void printClusterPath(std::ostream& os, const size_t clusterId);
    void dumpAllNodesMap();
    void setDebug(const bool enable);

    // all cluster markers IDs
    void getAllIds(const size_t clusterId, std::vector<size_t>& childPoiIds);

private:
    Q_DISABLE_COPY(PlacemarkSetLayer)
    void onProjection(QGVMap* geoMap) override;
    void onCamera(const QGVCameraState& oldState, const QGVCameraState& newState) override;
    void onUpdate() override;
    void onClean() override;

    void update() const;

    struct Internals;
    Internals* mInternals;
};
