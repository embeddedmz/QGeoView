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

#include "placemarkSet.h"

#include <cmath>
#include <set>
#include <unordered_map>

namespace
{
    const auto SQRT_TWO = std::sqrt(2.0);


}

// Internal class for cluster tree nodes
// Each node represents either one marker or a cluster of nodes
struct ClusteringNode
{
    size_t nodeId;
    size_t level; // for dev
    double gcsCoords[3]; // QGV::GeoPos ?
    ClusteringNode* parent;
    std::set<ClusteringNode*> children;
    size_t numberOfMarkers; // 1 for single-point nodes, > 1 for clusters
    size_t markerId;        // only relevant for single-point markers (not clusters)
    size_t numberOfVisibleMarkers;
    size_t numberOfSelectedMarkers;
};

struct PlacemarkSet::Internals
{
    // Used when rebuilding clustering tree
    void insertIntoNodeTable(ClusteringNode* node);

    // Computes squared clustering distance in gcs coordinates
    double computeDistanceThreshold2(const QGV::GeoPos& geoPos, const size_t clusteringDistance);

    // Find closest node within distance threshold squared
    ClusteringNode* FindClosestNode(ClusteringNode* node, const size_t zoomLevel,
        const double distanceThreshold2);
    void MergeNodes(ClusteringNode* node, ClusteringNode* mergingNode,
                    std::set<ClusteringNode*>& parentsToMerge, const size_t level);

    void GetMarkerIdsRecursive(const size_t clusterId, std::set<ClusteringNode*>& markerIds);

    const unsigned int BaseMarkerSize = 50;
    unsigned int PointMarkerSize = 50;
    unsigned int ClusterMarkerSize = 50;

    bool Initialized;

    bool Clustering;

    int ClusterDistance;

    QPixmap MarkerShape;

    size_t ZoomLevel; // Used for marker clustering

    size_t UniqueMarkerId;
    size_t UniqueNodeId;

    // index: displayId (marker in the map), used to handle selections
    std::vector<ClusteringNode*> CurrentNodes;

    // index: zoom level
    std::vector<std::set<ClusteringNode*>> NodeTable; // Used for marker clustering

    // for single-markers only (not clusters)
    // key: markerId
    std::unordered_map<size_t, bool> MarkerVisibleMap;
    std::unordered_map<size_t, bool> MarkerSelectedMap;

    // key: nodeID, the nodes already exist in NodeTable !
    std::unordered_map<size_t, ClusteringNode*> AllNodesMap; // for dev

    // key: markerID, the markers already exist in NodeTable's last set.
    // In fact, NodeTable last set, corresponding to the deepest zoom level
    // contains only markers !
    std::unordered_map<size_t, ClusteringNode*> MarkerNodesMap;


};

PlacemarkSet::PlacemarkSet()
{
    setFlag(QGV::ItemFlag::IgnoreScale);
    setFlag(QGV::ItemFlag::IgnoreAzimuth);
    setFlag(QGV::ItemFlag::Highlightable);
    setFlag(QGV::ItemFlag::HighlightCustom);
    setFlag(QGV::ItemFlag::Highlightable);
    setFlag(QGV::ItemFlag::Transformed);
    //setGeometry(geoPos, QSize(32, 32), QPoint(16, 32));
    const QString url = "http://maps.google.com/mapfiles/kml/paddle/blu-circle.png";
    load(url);
}

PlacemarkSet::~PlacemarkSet()
{

}

void PlacemarkSet::setImage(const QPixmap& img)
{

}

void PlacemarkSet::setClustering(const bool enable)
{

}

void PlacemarkSet::setClusteringTreeDepth(const size_t depth)
{

}

void PlacemarkSet::setClusterDistance(const size_t distance)
{

}

void PlacemarkSet::RecomputeClusters()
{

}

size_t PlacemarkSet::getNumberOfMarkers() const
{
    return 0;
}

size_t PlacemarkSet::add(const QGV::GeoPos& pos)
{
    return 0;
}
bool PlacemarkSet::remove(const size_t poiId)
{
    return true;
}

void PlacemarkSet::removeAll()
{

}

bool PlacemarkSet::setVisibility(const size_t poiId, const bool visible)
{
    return true;
}

bool PlacemarkSet::getVisibility(const size_t poiId) const
{
    return true;
}

void PlacemarkSet::getClusterChildren(const size_t clusterId,
                                      std::vector<size_t>& childPoiIds,
                                      std::vector<size_t>& childClusterIds)
{

}

void PlacemarkSet::getAllIds(const size_t clusterId, std::vector<size_t>& childPoiIds)
{

}

/* debug */
void PlacemarkSet::printClusterPath(std::ostream& os, const size_t clusterId)
{

}

void PlacemarkSet::dumpAllNodesMap()
{

}

QTransform PlacemarkSet::projTransform() const
{
    return isFlag(QGV::ItemFlag::Highlighted) ? QGV::createTransfromScale(projAnchor(), 1.2) : QTransform();
}

void PlacemarkSet::projOnFlags()
{
    setOpacity(isFlag(QGV::ItemFlag::Highlighted) ? 0.3 : 1.0);
}




// Used when rebuilding clustering tree
void PlacemarkSet::Internals::insertIntoNodeTable(ClusteringNode* node)
{

}

// Computes squared clustering distance in gcs coordinates
double PlacemarkSet::Internals::computeDistanceThreshold2(const QGV::GeoPos& geoPos, const size_t clusteringDistance)
{
    return 0;
}

// Find closest node within distance threshold squared
ClusteringNode* PlacemarkSet::Internals::FindClosestNode(ClusteringNode* node,
    const size_t zoomLevel,
    const double distanceThreshold2)
{
    return nullptr;
}

void PlacemarkSet::Internals::MergeNodes(ClusteringNode* node,
    ClusteringNode* mergingNode,
    std::set<ClusteringNode*>& parentsToMerge,
    const size_t level)
{

}

void PlacemarkSet::Internals::GetMarkerIdsRecursive(const size_t clusterId,
    std::set<ClusteringNode*>& markerIds)
{

}
