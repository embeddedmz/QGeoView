#include "colorMap.h"
#include "colormapPresetToPixmap.h"

#include <algorithm>

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QSize>

ColorMapPresetToPixmap::ColorMapPresetToPixmap(QObject* parentObject) :
    Superclass(parentObject)
{
}

QPixmap ColorMapPresetToPixmap::render(const ColorMapPresets::ControlPoints& controlPoints, const QSize& resolution) const
{
    if (resolution.width() <= 0 || resolution.height() <= 0)
    {
        return QPixmap();
    }

    LinearColorMap cm = ColorMapPresets::controlPointsToLinearColorMap(controlPoints);
    QImage image(256, 1, QImage::Format_ARGB32);
    QVector<QRgb> colorTable = cm.colorTable(0, 255);
    image.setColorTable(colorTable);

    for (int cc = 0; cc < colorTable.size(); ++cc)
    {
        image.setPixel(cc, 0, colorTable[cc]);
    }

    return QPixmap::fromImage(image.scaled(resolution));
}
