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

#include <QGeoView/QGVWidget.h>

#include "colorMap.h"

class QGVWidgetColorBar : public QGVWidget
{
    Q_OBJECT

public:
    QGVWidgetColorBar(Qt::Orientation orientation = Qt::Vertical);
    ~QGVWidgetColorBar() override;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation getOrientation() const;

    void setWidth(const int width);
    int getWidth() const;

    void setColorMap(const double min, const double max, const LinearColorMap& lcm);
    LinearColorMap getColorMap() const;
    double getMin() const;
    double getMax() const;

private:
    void paintEvent(QPaintEvent* event) override;

private:
    struct Internals;
    Internals* const mInternals;
};
