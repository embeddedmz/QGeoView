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

#include "pathColoring.h"

#include "samples/colorMap.h"
#include "samples/colorMapPresetDialog.h"
#include "samples/placemark.h"
#include "samples/polyline.h"
#include "samples/rescaleRangeDialog.h"

PathColoringDemo::PathColoringDemo(QGVMap* geoMap, QObject* parent)
    : DemoItem(geoMap, SelectorDialog::Single, parent)
{}

QString PathColoringDemo::label() const
{
    return "Background maps";
}

QString PathColoringDemo::comment() const
{
    return "QGV supports multiple tile map types. This includes:<br>"
           "- OpenStreetMaps<br>"
           "- Google Maps<br>"
           "- Bing Maps<br>"
           "- Custom maps(OSM-like, for e.g MapServer)";
}

void PathColoringDemo::onInit()
{
    QVector<QGV::GeoPos> linePoints{
        QGV::GeoPos{ 43.28849853885284, -0.40097961338582416 },
        QGV::GeoPos{ 43.288607663101814, -0.4011056068729572 },
        QGV::GeoPos{ 43.28870169558679, -0.4012124625167549 },
        QGV::GeoPos{ 43.288837520817815, -0.40126349922217774 },
        QGV::GeoPos{ 43.289008175284756, -0.40080258793938295 },
    };
    geoMap()->addItem(new Polyline(linePoints, Qt::GlobalColor::red));
    geoMap()->addItem(new Placemark(QGV::GeoPos(43.28885725761855, -0.40090465730287766)));

// plus tard...
// ui->geoMap->addWidget(new QGVWidgetColorBar());

    LinearColorMap lcm = ColorMapPresets::controlPointsToLinearColorMap(ColorMapPresets::Jet());

    ColorMapPresetDialog dialog(geoMap());
    dialog.setCurrentControlPoints(ColorMapPresets::Jet());
    // QObject::connect(&dialog, &ColorMapPresetDialog::presetApplied, [this]()
    //{ });
    dialog.exec();

    // retrieve the original range
    double rangeMin = 0;
    double rangeMax = 0;

    if (rangeMin == rangeMax) {
        ++rangeMax; // to avoid rescale failing
    }

    // show a dialog box
    RescaleRangeDialog rescaleDlg(geoMap());
    rescaleDlg.setRange(rangeMin, rangeMax);
    if (rescaleDlg.exec() == QDialog::Accepted) {
        const double dialogMinRange = rescaleDlg.minimum();
        const double dialogMaxRange = rescaleDlg.maximum();
        if (dialogMinRange != rangeMin || dialogMaxRange != rangeMax) {
            // ....
        }
    }

    selector()->select(0);
}

void PathColoringDemo::onStart()
{
    selector()->show();
}

void PathColoringDemo::onEnd()
{
    selector()->hide();
}

void PathColoringDemo::setSelected(void* something, bool selected)
{
    if (something == nullptr) {
        return;
    }
}
