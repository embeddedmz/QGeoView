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

#include "clusteringMarkersDemo.h"
#include "placemarkSetLayer.h"

#include <QPainter>
#include <QPixmap>

ClusteringMarkersDemo::ClusteringMarkersDemo(QGVMap* geoMap, QObject* parent)
    : DemoItem(geoMap, SelectorDialog::Multi, parent)
{}

QString ClusteringMarkersDemo::label() const
{
    return "Clustering POIs";
}

QString ClusteringMarkersDemo::comment() const
{
    return "TODO : infos on the used clustering...<br>"
           "<br>";
}

void ClusteringMarkersDemo::onInit()
{
    // QString url = R"(C:\\Users\\Amine Mzoughi\\Desktop\\blu-circle.png)";
    // QString url = R"(C:\\Users\\mmzoughi\\Pictures\\blu-circle.png)";
    // QPixmap pix(url);

    QPixmap pix(64, 64);
    QPainter paint(&pix);
    QPolygon triangle = QVector<QPoint>{ QPoint{ 0, 0 }, QPoint{ 32, 63 }, QPoint{ 63, 0 } };
    paint.setPen(QPen(QBrush(Qt::GlobalColor::red),
                      1,
                      Qt::PenStyle::SolidLine,
                      Qt::PenCapStyle::RoundCap,
                      Qt::PenJoinStyle::RoundJoin));
    paint.setBrush(QBrush(Qt::GlobalColor::red));
    paint.drawPolygon(triangle, Qt::WindingFill);
    paint.end();

    PlacemarkSetLayer* myPOIs = new PlacemarkSetLayer();
    // faudra le faire avant d'ajouter les POIs ou bien il faudra passer la geomap au ctor
    // ou bien changer l'archi de la couche
    geoMap()->addItem(myPOIs);
    myPOIs->setClustering(true);
    myPOIs->setClusteringTreeDepth(20);
    // myPOIs->setClustering(false);
    myPOIs->setImage(pix);
    // myPOIs->add(QGV::GeoPos{ 0, 0 });
    myPOIs->add(QGV::GeoPos{ 43.28849853885284, -0.40097961338582416 });
    myPOIs->add(QGV::GeoPos{ 43.288607663101814, -0.4011056068729572 });
    myPOIs->add(QGV::GeoPos{ 43.28870169558679, -0.4012124625167549 });
    myPOIs->add(QGV::GeoPos{ 43.288837520817815, -0.40126349922217774 });
    myPOIs->add(QGV::GeoPos{ 43.289008175284756, -0.40080258793938295 });

    selector()->addItem("Clustering Markers Demo", std::bind(&ClusteringMarkersDemo::setSelected, this, nullptr, std::placeholders::_1));
    selector()->selectAll();
}

void ClusteringMarkersDemo::onStart()
{
    selector()->show();
    geoMap()->flyTo(QGVCameraActions(geoMap()).scaleTo(targetAreaIn()));
}

void ClusteringMarkersDemo::onEnd()
{
    selector()->hide();
}

void ClusteringMarkersDemo::setSelected(void* something, bool selected)
{
    if (something == nullptr) {
        return;
    }
    if (selected) {
    
    }
}

QGV::GeoRect ClusteringMarkersDemo::targetAreaIn() const
{
    return QGV::GeoRect(QGV::GeoPos(48.236117, 11.499786), QGV::GeoPos(48.061851, 11.637178));
}
