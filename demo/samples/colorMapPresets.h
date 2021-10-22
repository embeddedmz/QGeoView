#pragma once

#include "colorMap.h"

#include <tuple>
#include <vector>

namespace ColorMapPresets
{

/* x (control point), red, green, blue
 * all values must be defined between 0.0 and 1.0
 * ControlPoints must be sorted in ascending order of 'x' points.
 */
typedef std::tuple<double, double, double, double> ControlPoint;
typedef std::vector<ControlPoint> ControlPoints;

ControlPoints BlackBodyRadiation();
ControlPoints CoolToWarm();
ControlPoints Jet();
ControlPoints Grayscale();
ControlPoints XRay();

ColorMap* controlPointsToColorMap(const ControlPoints& ctrlPts);

}
