#ifndef WaterfallPresetToPixmap_h
#define WaterfallPresetToPixmap_h

#include "Waterfall/ColorMaps.h"

#include <QObject>
#include <QScopedPointer>

class QPixmap;
class QSize;

/**
* WaterfallPresetToPixmap is a helper class to generate QPixmap from a color/opacity
* color map. Use WaterfallPresetToPixmap::render() to generate a QPixmap for a color
* map.
*/
class WaterfallPresetToPixmap : public QObject
{
    Q_OBJECT
    typedef QObject Superclass;

public:
    WaterfallPresetToPixmap(QObject* parent = nullptr);

    /**
     * Render a color map to a pixmap for the given resolution.
     */
    QPixmap render(const ColorMaps::ControlPoints& controlPoints, const QSize& resolution) const;

private:
    Q_DISABLE_COPY(WaterfallPresetToPixmap)

};

#endif
