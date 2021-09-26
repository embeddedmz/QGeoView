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

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QtMath>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <set>
#include <unordered_map>

namespace
{
    const auto SQRT_TWO = std::sqrt(2.0);

    int scaleToZoom(double scale)
    {
        const double scaleChange = 1 / scale;
        const int newZoom = qRound((17.0 - qLn(scaleChange) * M_LOG2E));
        return newZoom;
    }
}

// Internal class for cluster tree nodes
// Each node represents either one marker or a cluster of nodes
struct ClusteringNode
{
    size_t nodeId;
    int level; // for dev
    QGV::GeoPos geoCoords;
    QPointF gcsCoords;
    ClusteringNode* parent;
    std::set<ClusteringNode*> children;
    size_t numberOfMarkers; // 1 for single-point nodes, > 1 for clusters
    size_t markerId;        // only relevant for single-point markers (not clusters)
    size_t numberOfVisibleMarkers;
    size_t numberOfSelectedMarkers;
};

struct ClusterDrawingInformations
{
    QPointF postion;
    size_t poiCount = 0;
};

struct PlacemarkSet::Internals
{
    Internals(QGVMap* map)
        : gvMap(map)
    {
    }

    // Used when rebuilding clustering tree
    void insertIntoNodeTable(ClusteringNode* node);

    // Find closest node within distance threshold squared
    ClusteringNode* findClosestNode(ClusteringNode* node, const int zoomLevel);
    void mergeNodes(ClusteringNode* node, ClusteringNode* mergingNode,
                    std::set<ClusteringNode*>& parentsToMerge, const int level);

    void getMarkerIdsRecursive(const size_t clusterId, std::vector<size_t>& markerIds);
    void getClusterChildren(const size_t clusterId, std::vector<size_t>& childPoiIds,
        std::vector<size_t>& childClusterIds);

    const unsigned int BaseMarkerSize = 50;
    unsigned int PointMarkerSize = 50;
    unsigned int ClusterMarkerSize = 50;

    bool Clustering;
    
    size_t ClusteringTreeDepth;
    
    size_t ClusterDistance;

    QPixmap MarkerShape;

    int ZoomLevel; // Used for marker clustering

    // will be used to generate new IDs
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

    // Use for drawing
    //QVector<QGV::GeoPos> mPointsGeoPosList;
    QPolygonF mProjPoints;
    QPolygonF mProjClustersPolygon;
    std::vector<ClusterDrawingInformations> mProjClusters;

    bool Debug;

    QPointer<QGVMap> gvMap;
};

PlacemarkSet::PlacemarkSet(QGVMap* geoMap)
    : mInternals(new PlacemarkSet::Internals(geoMap))
    , mGeoMap(geoMap)
{
    Q_ASSERT(geoMap != nullptr);

    setSelectable(false);

    //mPointsGeoPosList.push_back(QGV::GeoPos(coordinates.at(1).toDouble(), coordinates.at(0).toDouble()));
    
    mInternals->Clustering = false;
    mInternals->ClusteringTreeDepth = 14;
    mInternals->ClusterDistance = 40;
    mInternals->Debug = false;

    mInternals->ZoomLevel = -1;
    mInternals->NodeTable.resize(mInternals->ClusteringTreeDepth);
    mInternals->UniqueMarkerId = 0;
    mInternals->UniqueNodeId = 0;

}

PlacemarkSet::~PlacemarkSet()
{
    removeAll();

    delete mInternals;
}

void PlacemarkSet::setImage(const QPixmap& img)
{
    mInternals->MarkerShape = img;
}

void PlacemarkSet::setClustering(const bool enable)
{
    mInternals->Clustering = enable;
}

void PlacemarkSet::setClusteringTreeDepth(size_t depth)
{
    // clamp between 2 and 20
    if (depth < 2)
        depth = 2;
    else if (depth > 20)
        depth = 20;

    mInternals->ClusteringTreeDepth = depth;
}

void PlacemarkSet::setClusterDistance(const size_t distance)
{
    mInternals->ClusterDistance = distance;
}

size_t PlacemarkSet::getNumberOfMarkers() const
{
    return mInternals->MarkerNodesMap.size();
}

size_t PlacemarkSet::add(const QGV::GeoPos& pos)
{
    // Set marker id
    const size_t markerId = mInternals->UniqueMarkerId++;

    if (mInternals->Debug)
        printf("[Debug] Adding marker %zu.\n", markerId);

    // Insert nodes at bottom level
    const int level = int(mInternals->NodeTable.size() - 1);

    // Allocate memory for a ClusteringNode
    ClusteringNode* const node = new ClusteringNode;

    // first node of this marker in the node's tree (zoom level 13)
    const size_t nodeId = mInternals->UniqueNodeId++;

    // check unicity of nodeId and markerId
    Q_ASSERT(mInternals->AllNodesMap.find(nodeId) == mInternals->AllNodesMap.end());
    Q_ASSERT(mInternals->MarkerVisibleMap.find(markerId) == mInternals->MarkerVisibleMap.end());
    Q_ASSERT(mInternals->MarkerSelectedMap.find(markerId) == mInternals->MarkerSelectedMap.end());
    Q_ASSERT(mInternals->MarkerNodesMap.find(markerId) == mInternals->MarkerNodesMap.end());

    mInternals->AllNodesMap.emplace(nodeId, node);
    node->nodeId = nodeId;
    node->level = level;
    
    node->geoCoords = pos;
    // Ask Andrey if this is correct :
    node->gcsCoords = mGeoMap->getProjection()->geoToProj(pos);
    // In VTK we do this :
    /* node->gcsCoords[0] = longitude;
    node->gcsCoords[1] = vtkMercator::lat2y(latitude);*/
    
    node->numberOfMarkers = 1;
    node->parent = nullptr;
    node->markerId = markerId;
    node->numberOfVisibleMarkers = 1;
    node->numberOfSelectedMarkers = 0;

    if (mInternals->Debug)
        printf("[Debug] Inserting a clustering node '%zu' into level '%d'.\n", node->nodeId, level);

    mInternals->NodeTable[size_t(level)].insert(node);
    mInternals->MarkerVisibleMap.emplace(markerId, true);
    mInternals->MarkerSelectedMap.emplace(markerId, false);
    mInternals->MarkerNodesMap.emplace(markerId, node);

    // For now, always insert into cluster tree even if clustering is disabled
    // We must do it now and we can't wait to do it in void PlacemarkSet::onProjection(QGVMap* geoMap)
    // to be able to determine new clusters !
    mInternals->insertIntoNodeTable(node);
    
    // TODO : notify that a new POI has been added to the set

    //dumpAllNodesMap();

    return markerId;
}

bool PlacemarkSet::remove(const size_t markerId)
{
    if (mInternals->MarkerNodesMap.find(markerId) == mInternals->MarkerNodesMap.end())
    {
        if (mInternals->Debug)
            printf("[Debug] DeleteMarker: Marker '%zu' doesn't exist !\n", markerId);

        return false;
    }

    ClusteringNode* const markerNode = mInternals->MarkerNodesMap[markerId];

    Q_ASSERT(mInternals->MarkerVisibleMap.find(markerId) != mInternals->MarkerVisibleMap.end());
    Q_ASSERT(mInternals->MarkerSelectedMap.find(markerId) != mInternals->MarkerSelectedMap.end());

    // Recursively update ancestors (ClusteringNode instances)
    int deltaVisible = mInternals->MarkerVisibleMap[markerId] ? 1 : 0;
    int deltaSelected = mInternals->MarkerSelectedMap[markerId] ? 1 : 0;

    ClusteringNode* node = markerNode;
    ClusteringNode* parent = node->parent;

    Q_ASSERT(node->numberOfMarkers >= 1);

    // First of all, we remove the marker from its Parent's Children set
    if (parent)
    {
        parent->children.erase(node);
    }

    while (parent)
    {
        // Erase node if it is empty
        if (node->numberOfMarkers < 1)
        {
            if (mInternals->Debug)
                printf("[Debug] Deleting node '%zu' level '%d'\n", node->nodeId, node->level);

            parent->children.erase(node);
            int level = node->level;

            mInternals->NodeTable[size_t(level)].erase(node);

            Q_ASSERT(mInternals->AllNodesMap.find(node->nodeId) != mInternals->AllNodesMap.end());

            mInternals->AllNodesMap.erase(node->nodeId);

            delete node;
            node = nullptr;
        }

        if (parent->numberOfMarkers > 1)
        {
            // Update gcsCoords
            double denom = static_cast<double>(parent->numberOfMarkers - 1);
            
            double num = (parent->numberOfMarkers * parent->gcsCoords.x()) - markerNode->gcsCoords.x();
            parent->gcsCoords.setX(num / denom);
            num = (parent->numberOfMarkers * parent->gcsCoords.y()) - markerNode->gcsCoords.y();
            parent->gcsCoords.setY(num / denom);
        }

        parent->numberOfMarkers -= 1;

        if (parent->numberOfMarkers == 1 && !parent->children.empty())
        {
            // Get MarkerId from remaining node
            std::set<ClusteringNode*>::iterator iter = parent->children.begin();
            ClusteringNode* const extantNode = *iter;
            parent->markerId = extantNode->markerId;
        }

        parent->numberOfVisibleMarkers -= deltaVisible;
        parent->numberOfSelectedMarkers -= deltaSelected;

        // Setup next iteration
        node = parent;
        parent = parent->parent;
    }

    // delete last node (at level 0)
    if (node && node->parent == nullptr && node->numberOfMarkers == 0) {
        const size_t indexInNodeTable = size_t(node->level);

        Q_ASSERT(mInternals->NodeTable[indexInNodeTable].find(node) !=
               mInternals->NodeTable[indexInNodeTable].end());

        if (indexInNodeTable < mInternals->NodeTable.size()) {
            mInternals->NodeTable[indexInNodeTable].erase(node);
        }

        Q_ASSERT(mInternals->AllNodesMap.find(node->nodeId) != mInternals->AllNodesMap.end());

        mInternals->AllNodesMap.erase(node->nodeId);

        delete node;
        node = nullptr;
    }

    // Update Internals and delete marker itself
    mInternals->AllNodesMap.erase(markerNode->nodeId);
    mInternals->MarkerNodesMap.erase(markerId);
    mInternals->MarkerVisibleMap.erase(markerId);
    mInternals->MarkerSelectedMap.erase(markerId);
    // delete marker (at the end of the node table)
    mInternals->NodeTable.back().erase(markerNode);

    if (mInternals->Debug)
        printf("[Debug] Deleting marker '%zu'\n", markerNode->markerId);
    
    delete markerNode; // free memory occupied by the marker's node
    //dumpAllNodesMap();

    // TODO : notify that a POI has been deleted from the set

    return true;
}

void PlacemarkSet::removeAll()
{
    mInternals->CurrentNodes.clear(); // to avoid selecting deleted nodes

    // NodeTable contains all markers/clusters nodes
    for (auto& nodesSet : mInternals->NodeTable)
    {
        for (auto node : nodesSet)
        {
            delete node;
        }
    }
    mInternals->NodeTable.clear();
    mInternals->NodeTable.resize(mInternals->ClusteringTreeDepth);

    mInternals->MarkerVisibleMap.clear();
    mInternals->MarkerSelectedMap.clear();

    mInternals->AllNodesMap.clear();
    mInternals->MarkerNodesMap.clear();

    mInternals->UniqueMarkerId = 0;
    mInternals->UniqueNodeId = 0;

    // TODO : notify that all POIs have been deleted from the set so that we can request a redraw of the map

}

void PlacemarkSet::recomputeClusters()
{
    // std::cout << "Enter recomputeClusters()" << std::endl;
    // Clear current data

    // delete all nodes except markers, their pointers are still
    // stored in MarkerNodesMap.
    for (size_t i = 0; i < mInternals->NodeTable.size() - 1; ++i)
    {
        std::set<ClusteringNode*>& clusterSet = mInternals->NodeTable[i];
        std::set<ClusteringNode*>::iterator clusterIter = clusterSet.begin();
        for (; clusterIter != clusterSet.end(); ++clusterIter)
        {
            ClusteringNode* cluster = *clusterIter;
            delete cluster;
        }
    }
    mInternals->NodeTable.clear();
    mInternals->AllNodesMap.clear();

    // Re-initialize node table
    mInternals->NodeTable.resize(mInternals->ClusteringTreeDepth);

    // Keep markers ID, do not reset markers unique ID "generators"
    // as user's application might hold markers IDs and thus it will
    // be unable to delete them via DeleteMarker or even designate them
    // correctly.
    // this->Internals->UniqueMarkerId = 0;
    mInternals->UniqueNodeId = 0;

    // Add marker nodes back into node table
    auto markerIter = mInternals->MarkerNodesMap.begin();
    for (; markerIter != mInternals->MarkerNodesMap.end(); ++markerIter)
    {
        ClusteringNode* const markerNode = markerIter->second;
        const auto nodeId = mInternals->UniqueNodeId++;

        markerNode->nodeId = nodeId;
        markerNode->level = mInternals->NodeTable.size() - 1;

        mInternals->NodeTable.back().insert(markerNode);
        mInternals->AllNodesMap.emplace(nodeId, markerNode);

        mInternals->insertIntoNodeTable(markerNode);
    }

    // TODO : notify that clusters has been recomputed so that we can request a redraw of the map


    // old stuff :
    // assert(this->Internals->NodeTable.back().size() ==
    //       this->Internals->MarkerNodesMap.size());
    // assert(this->Internals->MarkerNodesMap.size() ==
    //       this->Internals->MarkerVisibleMap.size());
    // assert(this->Internals->MarkerSelectedMap.size() ==
    //       this->Internals->MarkerVisibleMap.size());
    // size_t nodesCount = 0;
    // for (auto& nodesSet : this->Internals->NodeTable)
    //{
    //  nodesCount += nodesSet.size();
    //}
    // assert(nodesCount == this->Internals->AllNodesMap.size());

    // Sanity check node table
    // tableIter = this->Internals->NodeTable.begin();
    // for (auto i=0u; i < this->ClusteringTreeDepth; ++i, ++tableIter)
    //   {
    //   const std::set<ClusteringNode*>& clusterSet = *tableIter;
    //   std::cout << "Level " << i << " node count " << clusterSet.size() << std::endl;
    //   }
}

bool PlacemarkSet::setVisibility(const size_t poiId, const bool visible)
{
    // std::cout << "Set marker id " << markerId
    //           << " to visible: " << visible << std::endl;
    if (mInternals->MarkerNodesMap.find(poiId) == mInternals->MarkerNodesMap.end())
    {
        printf("[Warning] Invalid Marker Id '%zu'\n", poiId);

        return false;
    }

    Q_ASSERT(mInternals->MarkerVisibleMap.find(poiId) != mInternals->MarkerVisibleMap.end());

    if (visible == mInternals->MarkerVisibleMap[poiId])
    {
        return false; // no change
    }

    // Check that node wasn't deleted
    ClusteringNode* const node = mInternals->MarkerNodesMap[poiId];

    // Update marker's node
    node->numberOfVisibleMarkers = visible ? 1 : 0;

    // Recursively update ancestor ClusteringNode instances
    int delta = visible ? 1 : -1;
    ClusteringNode* parent = node->parent;
    while (parent) {
        parent->numberOfVisibleMarkers += delta;
        parent = parent->parent;
    }

    mInternals->MarkerVisibleMap[poiId] = visible;

    // TODO : notify that the visibility has been changed so that we can request a redraw of the map

    
    return true;
}

bool PlacemarkSet::getVisibility(const size_t poiId) const
{
    return true;
}

/* debug */
void PlacemarkSet::printClusterPath(std::ostream& os, const size_t clusterId)
{
    // Gather up nodes in a list (bottom to top)
    std::vector<ClusteringNode*> nodeList;
    if (mInternals->MarkerNodesMap.find(clusterId) == mInternals->MarkerNodesMap.end()) {
        std::cerr << "[Warning] Cluster " << clusterId << " doesn't exist !" << std::endl;
        return;
    }

    ClusteringNode* markerNode = mInternals->MarkerNodesMap[clusterId];
    nodeList.push_back(markerNode);
    ClusteringNode* parent = markerNode->parent;
    while (parent) {
        nodeList.push_back(parent);
        parent = parent->parent;
    }

    // Write the list top to bottom (reverse order)
    os << "Level, NodeId, MarkerId, NumberOfVisibleMarkers" << '\n';
    std::vector<ClusteringNode*>::reverse_iterator iter = nodeList.rbegin();
    for (; iter != nodeList.rend(); ++iter) {
        ClusteringNode* node = *iter;
        os << std::setw(2) << node->level << "  " << std::setw(5) << node->nodeId << "  " << std::setw(5)
           << node->markerId << "  " << std::setw(4) << node->numberOfVisibleMarkers << '\n';
    }
}

void PlacemarkSet::dumpAllNodesMap()
{
    // Dump all nodes
    for (const auto& entry : mInternals->AllNodesMap)
    {
        ClusteringNode* currentNode = entry.second;
        std::cout << "Node " << entry.first << " has ";
        if (currentNode)
        {
            std::cout << currentNode->children.size() << " children, " << currentNode->numberOfMarkers
                      << " markers, and its marker id: " << currentNode->markerId;
        }
        else
        {
            // I don't think this is useful...
            std::cout << " been deleted";
        }
        std::cout << "\n";
    }
    std::cout << std::endl; // flush
}

void PlacemarkSet::setDebug(const bool enable)
{
    mInternals->Debug = enable;
}

// Used when rebuilding clustering tree
void PlacemarkSet::Internals::insertIntoNodeTable(ClusteringNode* node)
{
    int level = node->level - 1;
    for (; level >= 0; level--)
    {
        ClusteringNode* closest = findClosestNode(node, level);
        if (closest)
        {
            // Todo Update closest node with marker info <== is this still relevant, I don't think so
            if (Debug)
                printf("[Debug] Found closest node to '%zu' at '%zu'.\n", node->nodeId, closest->nodeId);

            // add a comment to explain this code (I don't really fully understand it, I know it's cluster position
            // but why the vtkMap developer chose that algorithm to compute cluster position)
            double denominator = 1.0 + closest->numberOfMarkers;          
            double numerator = closest->gcsCoords.x() * closest->numberOfMarkers + node->gcsCoords.x();
            closest->gcsCoords.setX(numerator / denominator);
            numerator = closest->gcsCoords.y() * closest->numberOfMarkers + node->gcsCoords.y();
            closest->gcsCoords.setY(numerator / denominator);

            closest->numberOfMarkers++;
            closest->numberOfVisibleMarkers++;
            closest->markerId = (size_t)-1; // it became a cluster, maybe we should change the type of marker to a signed type
            closest->children.insert(node);
            node->parent = closest;

            // Insertion step ends with first clustering
            node = closest;
            
            break; // NB: level will not be decreased
        }
        else
        {
            // Copy node and add to this level (we can make use of a copy assignement operator but we have to think carefully before)
            ClusteringNode* newNode = new ClusteringNode;
            const auto newNodeId = UniqueNodeId++;
            AllNodesMap.emplace(newNodeId, newNode);
            newNode->nodeId = newNodeId;
            newNode->level = level;
            newNode->gcsCoords = node->gcsCoords;
            newNode->numberOfMarkers = node->numberOfMarkers; // redudant as this info is also contained in newNode->children ?
            newNode->numberOfVisibleMarkers = node->numberOfVisibleMarkers;
            newNode->numberOfSelectedMarkers = node->numberOfSelectedMarkers;
            newNode->markerId = node->markerId;
            newNode->parent = nullptr;
            newNode->children.insert(node);
            NodeTable[size_t(level)].insert(newNode);

            if (Debug)
                printf("[Debug] Copying node '%zu' to a new one '%zu' for level '%d' \n",
                       node->nodeId,
                       newNode->nodeId,
                       level);

            node->parent = newNode;
            node = newNode;
        }
    }

    // Advance to next level up
    node = node->parent; // if it's null level is < 0 and the we will not enter in the while loop
    level--; // OK

    // Refinement step: Continue iterating up while
    // * Merge any nodes identified in previous iteration
    // * Update node coordinates
    // * Check for closest node
    std::set<ClusteringNode*> nodesToMerge;
    std::set<ClusteringNode*> parentsToMerge;
    while (level >= 0)
    {
        // Merge nodes identified in previous iteration
        std::set<ClusteringNode*>::iterator mergingNodeIter = nodesToMerge.begin();
        for (; mergingNodeIter != nodesToMerge.end(); mergingNodeIter++)
        {
            ClusteringNode* mergingNode = *mergingNodeIter;
            if (node == mergingNode)
            {
                printf("[Warning] Node & merging node are the same '%zu'\n", node->nodeId);
                Q_ASSERT(false);
            }
            else
            {
                if (Debug)
                    printf("[Debug] At level '%d', merging node '%p' into '%p'\n", level, mergingNode, node);

                mergeNodes(node, mergingNode, parentsToMerge, level);
            }
        }

        // Update coordinates? <== is this still relevant ? already done in mergeNodes IMHO

        // Update count
        int numMarkers = 0;
        int numSelectedMarkers = 0;
        int numVisibleMarkers = 0;
        double numerator[2];
        numerator[0] = numerator[1] = 0.0;
        std::set<ClusteringNode*>::iterator childIter = node->children.begin();
        for (; childIter != node->children.end(); childIter++) {
            ClusteringNode* child = *childIter;
            numMarkers += child->numberOfMarkers;
            numSelectedMarkers += child->numberOfSelectedMarkers;
            numVisibleMarkers += child->numberOfVisibleMarkers;

            numerator[0] += child->numberOfMarkers * child->gcsCoords.x();
            numerator[1] += child->numberOfMarkers * child->gcsCoords.y();
        }
        node->numberOfMarkers = numMarkers;
        node->numberOfSelectedMarkers = numSelectedMarkers;
        node->numberOfVisibleMarkers = numVisibleMarkers;
        if (numMarkers > 1)
        {
            node->markerId = (size_t)-1;
        }
        node->gcsCoords.setX(numerator[0] / numMarkers);
        node->gcsCoords.setY(numerator[1] / numMarkers);

        // Check for new clustering partner
        ClusteringNode* closest = findClosestNode(node, level);
        if (closest)
        {
            mergeNodes(node, closest, parentsToMerge, level);
        }

        // Setup for next iteration
        nodesToMerge.clear();
        nodesToMerge = parentsToMerge;
        parentsToMerge.clear();
        node = node->parent;
        level--;
    }
}

// Find closest node within distance threshold squared
ClusteringNode* PlacemarkSet::Internals::findClosestNode(ClusteringNode* node, const int zoomLevel)
{
    ClusteringNode* closestNode = nullptr;
    double closestDistance2 = ClusterDistance;
    std::set<ClusteringNode*>& nodeSet = NodeTable[zoomLevel];
    std::set<ClusteringNode*>::const_iterator setIter = nodeSet.cbegin();
    for (; setIter != nodeSet.cend(); setIter++)
    {
        ClusteringNode* other = *setIter;
        if (other == node)
        {
            continue;
        }

        double d2 = 0.0;
        double d1 = other->gcsCoords.x() - node->gcsCoords.x();
        d2 += d1 * d1;
        d1 = other->gcsCoords.y() - node->gcsCoords.y();
        d2 += d1 * d1;
        d2 *= gvMap->getCamera().scale();
        if (d2 <= closestDistance2)
        {
            closestNode = other;
            closestDistance2 = d2;
        }
    }

    return closestNode;
}

void PlacemarkSet::Internals::mergeNodes(ClusteringNode* node,
    ClusteringNode* mergingNode,
    std::set<ClusteringNode*>& parentsToMerge,
    const int level)
{
    if (Debug)
        printf("[Debug] Merging '%zu' into '%zu'\n", mergingNode->nodeId, node->nodeId);

    if (node->level != mergingNode->level)
    {
        printf("[Error] Node '%zu' and node '%zu' are not at the same level\n", node->nodeId, mergingNode->nodeId);
        Q_ASSERT(false);
    }

    // Update gcsCoords
    const size_t numMarkers = node->numberOfMarkers + mergingNode->numberOfMarkers;
    const double denominator = static_cast<double>(numMarkers);

    double numerator = node->gcsCoords.x() * node->numberOfMarkers +
        mergingNode->gcsCoords.x() * mergingNode->numberOfMarkers;
    node->gcsCoords.setX(numerator / denominator);
    numerator = node->gcsCoords.y() * node->numberOfMarkers +
        mergingNode->gcsCoords.y() * mergingNode->numberOfMarkers;
    node->gcsCoords.setY(numerator / denominator);

    node->numberOfMarkers = numMarkers;
    node->numberOfVisibleMarkers += mergingNode->numberOfVisibleMarkers;
    node->markerId = -1;

    // Update links to/from children of merging node
    // Make a working copy of the child set
    std::set<ClusteringNode*> childNodeSet(mergingNode->children);
    std::set<ClusteringNode*>::iterator childNodeIter = childNodeSet.begin();
    for (; childNodeIter != childNodeSet.end(); childNodeIter++) {
        ClusteringNode* childNode = *childNodeIter;
        node->children.insert(childNode);
        childNode->parent = node;
    }

    // Adjust parent marker counts
    // Todo recompute from children
    size_t n = mergingNode->numberOfMarkers;
    node->parent->numberOfMarkers += n;
    mergingNode->parent->numberOfMarkers -= n;

    // Remove mergingNode from its parent
    ClusteringNode* parent = mergingNode->parent;
    parent->children.erase(mergingNode);

    // Remember parent node if different than node's parent
    if (mergingNode->parent && mergingNode->parent != node->parent)
    {
        parentsToMerge.insert(mergingNode->parent);
    }

    // Delete mergingNode
    // todo only delete if valid level specified?
    auto count = NodeTable[size_t(level)].count(mergingNode);
    if (count == 1)
    {
        NodeTable[size_t(level)].erase(mergingNode);
    }
    else
    {
        printf("[Error] Node '%zu' not found at level '%d'\n", mergingNode->nodeId, level);
        Q_ASSERT(false);
    }

    AllNodesMap.erase(mergingNode->nodeId);

    // todo Check CurrentNodes too?
    delete mergingNode;
}

void PlacemarkSet::Internals::getMarkerIdsRecursive(const size_t clusterId, std::vector<size_t>& markerIds)
{
    // Get children markers & clusters
    std::vector<size_t> childMarkerIds;
    std::vector<size_t> childClusterIds;
    getClusterChildren(clusterId, childMarkerIds, childClusterIds);

    // Copy marker ids
    for (size_t i = 0; i < childMarkerIds.size(); i++)
    {
        markerIds.push_back(childMarkerIds[i]);
    }

    // Traverse cluster ids
    for (size_t j = 0; j < childClusterIds.size(); j++)
    {
        size_t childId = childClusterIds[j];
        getMarkerIdsRecursive(childId, markerIds);
    }
}

// TODO
void PlacemarkSet::onProjection(QGVMap* geoMap)
{
    QGVDrawItem::onProjection(geoMap);

    // already done elsewhere when adding POIs to this set
    /* if (!mInternals->mPointsGeoPosList.isEmpty()) {
        mInternals->mProjPoints.resize(mInternals->mPointsGeoPosList.size());
        for (int i = 0; i < mInternals->mPointsGeoPosList.size(); ++i) {
            mInternals->mProjPoints[i] = geoMap->getProjection()->geoToProj(mInternals->mPointsGeoPosList[i]);
        }
    }*/
}

// TODO...
QPainterPath PlacemarkSet::projShape() const
{
    update();

    QPainterPath path;
    path.addPolygon(mInternals->mProjPoints);
    path.addPolygon(mInternals->mProjClustersPolygon);
    return path;
}

// TODO...
void PlacemarkSet::projPaint(QPainter* painter)
{
    QBrush brush(Qt::GlobalColor::green);
    painter->setPen(QPen(brush, 1, Qt::PenStyle::SolidLine, Qt::PenCapStyle::RoundCap, Qt::PenJoinStyle::RoundJoin));
    painter->setBrush(QBrush(brush));

    // draw POIs
    for (const auto& poi : mInternals->mProjPoints)
    {
        painter->drawPixmap(poi, mInternals->MarkerShape);
    }

    // draw clusters
    for (const auto& clusterInfos : mInternals->mProjClusters)
    {
        painter->drawEllipse(clusterInfos.postion, 64, 64);
        
        const QString strPoiCount = QString::number(clusterInfos.poiCount);
        painter->setFont(QFont("Times", 10, QFont::Bold));
        QFontMetrics fm(painter->font());
        const int poiCountPixelSize = fm.width(strPoiCount);
        painter->drawText(clusterInfos.postion.x() - poiCountPixelSize / 2, clusterInfos.postion.y(), strPoiCount);
    }
}

QString PlacemarkSet::projTooltip(const QPointF& projPos) const
{
    auto geo = getMap()->getProjection()->projToGeo(projPos);
    return QString("Placemarker set\n"
        "\tClustering: %1\n"
        "\tClustering distance: %2 pixels\n"
        "\tNumber of markers : %3")
            .arg(mInternals->Clustering ? "On" : "Off")
            .arg(mInternals->ClusterDistance)
            .arg(mInternals->MarkerNodesMap.size());
}

void PlacemarkSet::Internals::getClusterChildren(const size_t clusterId,
                                                 std::vector<size_t>& childPoiIds,
                                                 std::vector<size_t>& childClusterIds)
{
    childPoiIds.clear();
    childClusterIds.clear();

    if (AllNodesMap.find(clusterId) == AllNodesMap.end()) {
        return;
    }

    // Check if node has been deleted, I don't think this is needed.
    ClusteringNode* node = AllNodesMap[clusterId];
    if (!node) {
        return;
    }

    std::set<ClusteringNode*>::iterator childIter = node->children.begin();
    for (; childIter != node->children.end(); childIter++) {
        ClusteringNode* child = *childIter;
        if (child->numberOfMarkers == 1) {
            childPoiIds.push_back(child->markerId);
        } else {
            childClusterIds.push_back(child->nodeId);
        } // else
    }     // for (childIter)
}

void PlacemarkSet::getAllIds(const size_t clusterId, std::vector<size_t>& childPoiIds)
{
    childPoiIds.clear();
    // Check if input id is marker

    if (mInternals->AllNodesMap.find(clusterId) == mInternals->AllNodesMap.end()) {
        return;
    }
    ClusteringNode* node = mInternals->AllNodesMap[clusterId];

    if (node->numberOfMarkers == 1) {
        childPoiIds.push_back(clusterId);
        return;
    }

    mInternals->getMarkerIdsRecursive(clusterId, childPoiIds);
}

void PlacemarkSet::update() const
{
    // 1. Get zoom level (0 based), clamp it if necessary
    // Clip zoom level to size of cluster table
    int zoomLevel = scaleToZoom(mGeoMap->getCamera().scale()) - 1;

    Q_ASSERT(zoomLevel >= 0);
    // clamp
    if (zoomLevel >= int(mInternals->NodeTable.size())) {
        zoomLevel = int(mInternals->NodeTable.size()) - 1;
    }

    // 2. Only need to rebuild geometrical data if either
    // 1. Contents have been modified
    // 2. In clustering mode and zoom level changed
    /* bool changed = this->GetMTime() > this->UpdateTime.GetMTime();
    changed |= this->Clustering && (zoomLevel != this->Internals->ZoomLevel);
    changed |= this->GetMTime() > this->Internals->ShapeInitTime;
    if (!changed) {
        return;
    }*/

    // In non-clustering mode, markers stored at leaf level
    /* if (!this->Clustering) {
        zoomLevel = int(this->Internals->NodeTable.size()) - 1;
    }*/

    // Copy marker info into polydata vtkNew<vtkPoints> points;

    // Get pointers to data arrays - clear points lists

    // Update visibility...

    // Another time :
    // Coefficients for scaling cluster size, using simple 2nd order model
    // The equation is y = k*x^2 / (x^2 + b), where k,b are coefficients
    // Logic hard-codes the min cluster factor to 1, i.e., y(2) = 1.0
    // Max value is k, which sets the horizontal asymptote :
    //const double k = this->MaxClusterScaleFactor;
    //const double b = 4.0 * k - 4.0;

    mInternals->CurrentNodes.clear();
    mInternals->mProjPoints.clear();
    mInternals->mProjClusters.clear();

    std::set<ClusteringNode*>& nodeSet = mInternals->NodeTable[size_t(zoomLevel)];
    std::set<ClusteringNode*>::const_iterator iter;
    for (iter = nodeSet.cbegin(); iter != nodeSet.cend(); iter++)
    {
        ClusteringNode* const node = *iter;
        if (!node->numberOfVisibleMarkers)
        {
            continue;
        }

        // Insert point
        //double z = node->gcsCoords[2] + (node->NumberOfSelectedMarkers ? this->SelectedZOffset : 0.0);
        // points->InsertNextPoint(node->gcsCoords[0], node->gcsCoords[1], z);
        mInternals->CurrentNodes.push_back(node);

        if (node->numberOfMarkers == 1)
        {
            mInternals->mProjPoints.push_back(node->gcsCoords);

            /* types->InsertNextValue(MARKER_TYPE);
            const auto map = this->Layer->GetMap();
            const double adjustedMarkerSize = map->GetDevicePixelRatio() * int(this->PointMarkerSize);
            const double markerScale = adjustedMarkerSize / this->BaseMarkerSize;
            scales->InsertNextValue(markerScale);*/
        }
        else if (mInternals->Clustering)
        {
            ClusterDrawingInformations cdi;
            cdi.poiCount = node->numberOfMarkers;
            cdi.postion = node->gcsCoords;
            mInternals->mProjClusters.push_back(cdi);
            mInternals->mProjClustersPolygon.push_back(cdi.postion);

            /*types->InsertNextValue(CLUSTER_TYPE);
            switch (this->ClusterMarkerSizeMode) {
                case POINTS_CONTAINED: {
                    // Scale with number of markers (quadratic model)
                    const double x = static_cast<double>(node->NumberOfMarkers);
                    const double scale = k * x * x / (x * x + b);
                    scales->InsertNextValue(scale);
                } break;

                case USER_DEFINED: {
                    // Scale with user defined size
                    const auto map = this->Layer->GetMap();
                    const double adjustedMarkerSize = map->GetDevicePixelRatio() * int(this->ClusterMarkerSize);
                    const double markerScale = adjustedMarkerSize / this->BaseMarkerSize;
                    scales->InsertNextValue(markerScale);
                } break;
            }*/
        }
        const size_t numMarkers = node->numberOfVisibleMarkers;

        // Set visibility
        const bool isVisible = numMarkers > 0;
        //visibles->InsertNextValue(isVisible);

        // Set label visibility
        const bool labelVis = numMarkers > 1;
        //labelVisArray->InsertNextValue(labelVis);

        // Set color
        const bool isSelected = node->numberOfSelectedMarkers > 0;
        //selects->InsertNextValue(isSelected);

        // Set number of markers
        //numMarkersArray->InsertNextValue(static_cast<const unsigned int>(numMarkers));
    }
    //this->PolyData->Reset();
    //this->PolyData->SetPoints(points.GetPointer());

    mInternals->ZoomLevel = zoomLevel;
    //this->UpdateTime.Modified();
}
