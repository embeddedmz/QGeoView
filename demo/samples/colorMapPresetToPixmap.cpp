#include "WaterfallPresetToPixmap.h"

#include <algorithm>
#include <qwt_color_map.h>

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QSize>

namespace
{

QwtColorMap* controlPointsToQwtColorMap(const ColorMaps::ControlPoints& ctrlPts)
{
    using namespace ColorMaps;

    if (ctrlPts.size() < 2 ||
        std::get<0>(ctrlPts.front()) != 0. ||
        std::get<0>(ctrlPts.back())  != 1. ||
        !std::is_sorted(ctrlPts.cbegin(), ctrlPts.cend(),
        [](const ControlPoint& x, const ControlPoint& y)
        {
            // strict weak ordering
            return std::get<0>(x) < std::get<0>(y);
        }))
    {
        return nullptr;
    }

    QColor from, to;
    from.setRgbF(std::get<1>(ctrlPts.front()), std::get<2>(ctrlPts.front()), std::get<3>(ctrlPts.front()));
    to.setRgbF(std::get<1>(ctrlPts.back()), std::get<2>(ctrlPts.back()), std::get<3>(ctrlPts.back()));

    QwtLinearColorMap* lcm = new QwtLinearColorMap(from, to, QwtColorMap::RGB);

    for (size_t i = 1; i < ctrlPts.size() - 1; ++i)
    {
        QColor cs;
        cs.setRgbF(std::get<1>(ctrlPts[i]), std::get<2>(ctrlPts[i]), std::get<3>(ctrlPts[i]));
        lcm->addColorStop(std::get<0>(ctrlPts[i]), cs);
    }

    return lcm;
}
}

WaterfallPresetToPixmap::WaterfallPresetToPixmap(QObject* parentObject) :
    Superclass(parentObject)
{
}

QPixmap WaterfallPresetToPixmap::render(const ColorMaps::ControlPoints& controlPoints,
                                        const QSize& resolution) const
{
    if (resolution.width() <= 0 || resolution.height() <= 0)
    {
        return QPixmap();
    }

    QScopedPointer<QwtColorMap> cm(controlPointsToQwtColorMap(controlPoints));

    QImage image(256, 1, QImage::Format_ARGB32);

    QVector<QRgb> colorTable = cm->colorTable(QwtInterval(0, 255));
    image.setColorTable(colorTable);

    for (int cc = 0; cc < colorTable.size(); ++cc)
    {
        image.setPixel(cc, 0, colorTable[cc]);
    }

    return QPixmap::fromImage(image.scaled(resolution));
}
