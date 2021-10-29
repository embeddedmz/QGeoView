#include "colorMapPresets.h"

#include <QColor>

namespace ColorMapPresets
{

LinearColorMap controlPointsToLinearColorMap(const ControlPoints& ctrlPts)
{
    if (ctrlPts.size() < 2 || std::get<0>(ctrlPts.front()) != 0. || std::get<0>(ctrlPts.back()) != 1. ||
        !std::is_sorted(ctrlPts.cbegin(), ctrlPts.cend(), [](const ControlPoint& x, const ControlPoint& y) {
            // strict weak ordering
            return std::get<0>(x) < std::get<0>(y);
        })) {
        QColor from, to;
        return LinearColorMap(from, to);
    }

    QColor from, to;
    from.setRgbF(std::get<1>(ctrlPts.front()), std::get<2>(ctrlPts.front()), std::get<3>(ctrlPts.front()));
    to.setRgbF(std::get<1>(ctrlPts.back()), std::get<2>(ctrlPts.back()), std::get<3>(ctrlPts.back()));

    LinearColorMap lcm(from, to);

    for (size_t i = 1; i < ctrlPts.size() - 1; ++i) {
        QColor cs;
        cs.setRgbF(std::get<1>(ctrlPts[i]), std::get<2>(ctrlPts[i]), std::get<3>(ctrlPts[i]));
        lcm.addColorStop(std::get<0>(ctrlPts[i]), cs);
    }

    return lcm;
}

ControlPoints BlackBodyRadiation()
{
    const double rgbPoints[] = {
        0,   0,              0,              0,
        0.4, 0.901960784314, 0,              0,
        0.8, 0.901960784314, 0.901960784314, 0,
        1,   1,              1,              1 };

    ControlPoints ctrlPts;
    for (size_t rgb = 0; rgb < sizeof(rgbPoints) / sizeof(double); rgb += 4)
    {
        ctrlPts.push_back(ControlPoint(rgbPoints[rgb],
                                       rgbPoints[rgb + 1],
                                       rgbPoints[rgb + 2],
                                       rgbPoints[rgb + 3]));
    }

    return ctrlPts;
}

ControlPoints CoolToWarm()
{
    const double rgbPoints[] = {
    0,   0.23137254902000001, 0.298039215686,       0.75294117647100001,
    0.5, 0.86499999999999999, 0.86499999999999999,  0.86499999999999999,
    1,   0.70588235294099999, 0.015686274509800001, 0.149019607843 };

    ControlPoints ctrlPts;
    for (size_t rgb = 0; rgb < sizeof(rgbPoints) / sizeof(double); rgb += 4)
    {
        ctrlPts.push_back(ControlPoint(rgbPoints[rgb],
                                       rgbPoints[rgb + 1],
                                       rgbPoints[rgb + 2],
                                       rgbPoints[rgb + 3]));
    }

    return ctrlPts;
}

ControlPoints Jet()
{
    QColor darkBlue(Qt::darkBlue); // 0.0
    QColor blue(Qt::blue);         // 0.2
    QColor cyan(Qt::cyan);         // 0.4
    QColor yellow(Qt::yellow);     // 0.6
    QColor red(Qt::red);           // 0.8
    QColor darkRed(Qt::darkRed);   // 1.0

    const double rgbPoints[] = {
    0.0, darkBlue.redF(), darkBlue.greenF(), darkBlue.blueF(),
    0.2, blue.redF(),     blue.greenF(),     blue.blueF(),
    0.4, cyan.redF(),     cyan.greenF(),     cyan.blueF(),
    0.6, yellow.redF(),   yellow.greenF(),   yellow.blueF(),
    0.8, red.redF(),      red.greenF(),      red.blueF(),
    1.0, darkRed.redF(),  darkRed.greenF(),  darkRed.blueF() };

    ControlPoints ctrlPts;
    for (size_t rgb = 0; rgb < sizeof(rgbPoints) / sizeof(double); rgb += 4)
    {
        ctrlPts.push_back(ControlPoint(rgbPoints[rgb],
                                       rgbPoints[rgb + 1],
                                       rgbPoints[rgb + 2],
                                       rgbPoints[rgb + 3]));
    }

    return ctrlPts;
}

ControlPoints Grayscale()
{
    const double rgbPoints[8] = {
        0, 0, 0, 0,
        1, 1, 1, 1 };

    ControlPoints ctrlPts;
    for (size_t rgb = 0; rgb < sizeof(rgbPoints)/sizeof(double); rgb += 4)
    {
        ctrlPts.push_back(ControlPoint(rgbPoints[rgb],
                                       rgbPoints[rgb + 1],
                                       rgbPoints[rgb + 2],
                                       rgbPoints[rgb + 3]));
    }

    return ctrlPts;
}

ControlPoints XRay()
{
    const double rgbPoints[] = {
        0, 1, 1, 1,
        1, 0, 0, 0 };

    ControlPoints ctrlPts;
    for (size_t rgb = 0; rgb < sizeof(rgbPoints)/sizeof(double); rgb += 4)
    {
        ctrlPts.push_back(ControlPoint(rgbPoints[rgb],
                                       rgbPoints[rgb + 1],
                                       rgbPoints[rgb + 2],
                                       rgbPoints[rgb + 3]));
    }

    return ctrlPts;
}

ColorMap* controlPointsToQwtColorMap(const ControlPoints& ctrlPts)
{
    if (ctrlPts.size() < 2 || std::get<0>(ctrlPts.front()) != 0. || std::get<0>(ctrlPts.back()) != 1. ||
        !std::is_sorted(ctrlPts.cbegin(), ctrlPts.cend(), [](const ControlPoint& x, const ControlPoint& y) {
            // strict weak ordering
            return std::get<0>(x) < std::get<0>(y);
        })) {
        return nullptr;
    }

    QColor from, to;
    from.setRgbF(std::get<1>(ctrlPts.front()), std::get<2>(ctrlPts.front()), std::get<3>(ctrlPts.front()));
    to.setRgbF(std::get<1>(ctrlPts.back()), std::get<2>(ctrlPts.back()), std::get<3>(ctrlPts.back()));

    LinearColorMap* lcm = new LinearColorMap(from, to);

    for (size_t i = 1; i < ctrlPts.size() - 1; ++i) {
        QColor cs;
        cs.setRgbF(std::get<1>(ctrlPts[i]), std::get<2>(ctrlPts[i]), std::get<3>(ctrlPts[i]));
        lcm->addColorStop(std::get<0>(ctrlPts[i]), cs);
    }

    return lcm;
}

}
