#include "colorMap.h"

/*!
   Map a value into a color

   \param interval Valid interval for values
   \param value Value

   \return Color corresponding to value

   \warning This method is slow for Indexed color maps. If it is
            necessary to map many values, its better to get the
            color table once and find the color using colorIndex().
*/
QColor ColorMap::color(const double min, const double max, double value) const
{
    return QColor::fromRgba(rgb(min, max, value));
}

/*!
   Build and return a color map of 256 colors

   The color table is needed for rendering indexed images in combination
   with using colorIndex().

   \param interval Range for the values
   \return A color table, that can be used for a QImage
*/
QVector<QRgb> ColorMap::colorTable(const double min, const double max) const
{
    QVector<QRgb> table(256);

    if (min <= max) {
        const double step = (max - min) / (table.size() - 1);
        for (int i = 0; i < table.size(); i++)
            table[i] = rgb(min, max, min + step * i);
    }

    return table;
}

/*!
   Build a color map with two stops at 0.0 and 1.0.

   \param color1 Color used for the minimum value of the value interval
   \param color2 Color used for the maximum value of the value interval
*/
LinearColorMap::LinearColorMap(const QColor& color1, const QColor& color2)
{
    d_mode = ScaledColors;
    setColorInterval(color1, color2);
}

/*!
   \brief Set the mode of the color map

   FixedColors means the color is calculated from the next lower
   color stop. ScaledColors means the color is calculated
   by interpolating the colors of the adjacent stops.

   \sa mode()
*/
void LinearColorMap::setMode(Mode mode)
{
    d_mode = mode;
}

/*!
   \return Mode of the color map
   \sa setMode()
*/
LinearColorMap::Mode LinearColorMap::mode() const
{
    return d_mode;
}

/*!
   Set the color range

   Add stops at 0.0 and 1.0.

   \param color1 Color used for the minimum value of the value interval
   \param color2 Color used for the maximum value of the value interval

   \sa color1(), color2()
*/
void LinearColorMap::setColorInterval(const QColor& color1, const QColor& color2)
{
    d_colorStops = ColorStops();
    d_colorStops.insert(0.0, color1);
    d_colorStops.insert(1.0, color2);
}

/*!
   Add a color stop

   The value has to be in the range [0.0, 1.0].
   F.e. a stop at position 17.0 for a range [10.0,20.0] must be
   passed as: (17.0 - 10.0) / (20.0 - 10.0)

   \param value Value between [0.0, 1.0]
   \param color Color stop
*/
void LinearColorMap::addColorStop(double value, const QColor& color)
{
    if (value >= 0.0 && value <= 1.0)
        d_colorStops.insert(value, color);
}

/*!
   \return Positions of color stops in increasing order
*/
QVector<double> LinearColorMap::colorStops() const
{
    return d_colorStops.stops();
}

/*!
  \return the first color of the color range
  \sa setColorInterval()
*/
QColor LinearColorMap::color1() const
{
    return QColor(d_colorStops.rgb(d_mode, 0.0));
}

/*!
  \return the second color of the color range
  \sa setColorInterval()
*/
QColor LinearColorMap::color2() const
{
    return QColor(d_colorStops.rgb(d_mode, 1.0));
}

/*!
  Map a value of a given interval into a RGB value

  \param interval Range for all values
  \param value Value to map into a RGB value

  \return RGB value for value
*/
QRgb LinearColorMap::rgb(const double min, const double max, double value) const
{
    if (qIsNaN(value))
        return 0u;

    const double width = max - min;
    if (width <= 0.0)
        return 0u;

    const double ratio = (value - min) / width;
    return d_colorStops.rgb(d_mode, ratio);
}

LinearColorMap::ColorStops::ColorStops() :
    d_doAlpha(false)
{
    d_stops.reserve(256);
}

void LinearColorMap::ColorStops::insert(double pos, const QColor& color)
{
    // Lookups need to be very fast, insertions are not so important.
    // Anyway, a balanced tree is what we need here. TODO ...

    if (pos < 0.0 || pos > 1.0)
        return;

    int index;
    if (d_stops.size() == 0) {
        index = 0;
        d_stops.resize(1);
    } else {
        index = findUpper(pos);
        if (index == d_stops.size() || qAbs(d_stops[index].pos - pos) >= 0.001) {
            d_stops.resize(d_stops.size() + 1);
            for (int i = d_stops.size() - 1; i > index; i--)
                d_stops[i] = d_stops[i - 1];
        }
    }

    d_stops[index] = ColorStop(pos, color);
    if (color.alpha() != 255)
        d_doAlpha = true;

    if (index > 0)
        d_stops[index - 1].updateSteps(d_stops[index]);

    if (index < d_stops.size() - 1)
        d_stops[index].updateSteps(d_stops[index + 1]);
}

QRgb LinearColorMap::ColorStops::rgb(LinearColorMap::Mode mode, double pos) const
{
    if (pos <= 0.0)
        return d_stops[0].rgb;
    if (pos >= 1.0)
        return d_stops[d_stops.size() - 1].rgb;

    const int index = findUpper(pos);
    if (mode == FixedColors) {
        return d_stops[index - 1].rgb;
    } else {
        const ColorStop& s1 = d_stops[index - 1];

        const double ratio = (pos - s1.pos) / (s1.posStep);

        const int r = int(s1.r0 + ratio * s1.rStep);
        const int g = int(s1.g0 + ratio * s1.gStep);
        const int b = int(s1.b0 + ratio * s1.bStep);

        if (d_doAlpha) {
            if (s1.aStep) {
                const int a = int(s1.a0 + ratio * s1.aStep);
                return qRgba(r, g, b, a);
            } else {
                return qRgba(r, g, b, s1.a);
            }
        } else {
            return qRgb(r, g, b);
        }
    }
}

QVector<double> LinearColorMap::ColorStops::stops() const
{
    QVector<double> positions(d_stops.size());
    for (int i = 0; i < d_stops.size(); i++)
        positions[i] = d_stops[i].pos;
    return positions;
}

int LinearColorMap::ColorStops::findUpper(double pos) const
{
    int index = 0;
    int n = d_stops.size();

    const ColorStop* stops = d_stops.data();

    while (n > 0) {
        const int half = n >> 1;
        const int middle = index + half;

        if (stops[middle].pos <= pos) {
            index = middle + 1;
            n -= half + 1;
        } else
            n = half;
    }

    return index;
}

LinearColorMap::ColorStops::ColorStop::ColorStop() :
    pos(0.0),
    rgb(0)
{

}

LinearColorMap::ColorStops::ColorStop::ColorStop(double p, const QColor& c)
    :
    pos(p),
    rgb(c.rgba())
{
    r = qRed(rgb);
    g = qGreen(rgb);
    b = qBlue(rgb);
    a = qAlpha(rgb);

    /*
        when mapping a value to rgb we will have to calcualate:
           - const int v = int( ( s1.v0 + ratio * s1.vStep ) + 0.5 );

        Thus adding 0.5 ( for rounding ) can be done in advance
     */
    r0 = r + 0.5;
    g0 = g + 0.5;
    b0 = b + 0.5;
    a0 = a + 0.5;

    rStep = gStep = bStep = aStep = 0.0;
    posStep = 0.0;
}

void LinearColorMap::ColorStops::ColorStop::updateSteps(const ColorStop& nextStop)
{
    rStep = nextStop.r - r;
    gStep = nextStop.g - g;
    bStep = nextStop.b - b;
    aStep = nextStop.a - a;
    posStep = nextStop.pos - pos;
}
