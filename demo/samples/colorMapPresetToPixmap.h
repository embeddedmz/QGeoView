#pragma once

#include "colorMapPresets.h"

#include <QObject>

class QPixmap;
class QSize;

/**
* ColorMapPresetToPixmap is a helper class to generate QPixmap from a color/opacity
* color map. Use ColorMapPresetToPixmap::render() to generate a QPixmap for a color
* map.
*/
class ColorMapPresetToPixmap : public QObject
{
    Q_OBJECT
    typedef QObject Superclass;

public:
    ColorMapPresetToPixmap(QObject* parent = nullptr);

    /**
     * Render a color map to a pixmap for the given resolution.
     */
    QPixmap render(const ColorMapPresets::ControlPoints& controlPoints, const QSize& resolution) const;

private:
    Q_DISABLE_COPY(ColorMapPresetToPixmap)

};
