#pragma once

#include "demoitem.h"

class PathColoringDemo : public DemoItem
{
    Q_OBJECT

public:
    explicit PathColoringDemo(QGVMap* geoMap, QObject* parent = 0);
};