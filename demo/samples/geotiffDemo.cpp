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

#include "geotiffDemo.h"
#include "samples/raster.h"

GeoTIFFDemo::GeoTIFFDemo(QGVMap* geoMap, QObject* parent)
    : DemoItem(geoMap, SelectorDialog::Multi, parent)
{}

QString GeoTIFFDemo::label() const
{
    return "Clustering POIs";
}

QString GeoTIFFDemo::comment() const
{
    return "TODO : infos on the used clustering...<br>"
           "<br>";
}

void GeoTIFFDemo::onInit()
{
    selector()->selectAll();
}

void GeoTIFFDemo::onStart()
{
    selector()->show();
    geoMap()->flyTo(QGVCameraActions(geoMap()).scaleTo(targetAreaIn()));
}

void GeoTIFFDemo::onEnd()
{
    selector()->hide();
}

void GeoTIFFDemo::setSelected(void* something, bool selected)
{
    if (something == nullptr) {
        return;
    }
    if (selected) {
    }
}

QGV::GeoRect GeoTIFFDemo::targetAreaIn() const
{
    return QGV::GeoRect(QGV::GeoPos(48.236117, 11.499786), QGV::GeoPos(48.061851, 11.637178));
}
