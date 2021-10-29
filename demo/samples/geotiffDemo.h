#pragma once

#include "demoitem.h"

class GeoTIFFDemo : public DemoItem
{
    Q_OBJECT

public:
    explicit GeoTIFFDemo(QGVMap* geoMap, QObject* parent = 0);
};