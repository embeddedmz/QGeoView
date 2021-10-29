#pragma once

#include <QColor>
#include <QVector>

/*!
  \brief ColorMap is used to map values into colors.

  For displaying 3D data on a 2D plane the 3rd dimension is often
  displayed using colors, like f.e in a spectrogram.
*/
class ColorMap
{
public:
    ColorMap() = default;
    virtual ~ColorMap() = default;

    /*!
       Map a value of a given interval into a RGB value.

       \param interval Range for the values
       \param value Value
       \return RGB value, corresponding to value
    */
    virtual QRgb rgb(const double min, const double max, double value) const = 0;

    QColor color(const double min, const double max, double value) const;
    virtual QVector<QRgb> colorTable(const double min, const double max) const;
};

/*!
  \brief LinearColorMap builds a color map from color stops.

  A color stop is a color at a specific position. The valid
  range for the positions is [0.0, 1.0]. When mapping a value
  into a color it is translated into this interval according to mode().
*/
class LinearColorMap : public ColorMap
{
public:
    /*!
       Mode of color map
       \sa setMode(), mode()
    */
    enum Mode
    {
        //! Return the color from the next lower color stop
        FixedColors,

        //! Interpolating the colors of the adjacent stops.
        ScaledColors
    };

    LinearColorMap(const QColor& color1, const QColor& color2);
    //~LinearColorMap() override;

    void setMode(Mode);
    Mode mode() const;

    void setColorInterval(const QColor& color1, const QColor& color2);
    void addColorStop(double value, const QColor&);
    QVector<double> colorStops() const;

    QColor color1() const;
    QColor color2() const;

    QRgb rgb(const double min, const double max, double value) const override;

    class ColorStops
    {
    public:
        ColorStops();

        void insert(double pos, const QColor& color);
        QRgb rgb(LinearColorMap::Mode, double pos) const;

        QVector<double> stops() const;

    private:
        struct ColorStop
        {
            ColorStop();
            ColorStop(double p, const QColor& c);
            void updateSteps(const ColorStop& nextStop);

            double pos;
            QRgb rgb;
            int r, g, b, a;

            // precalculated values
            double rStep, gStep, bStep, aStep;
            double r0, g0, b0, a0;
            double posStep;
        };

        int findUpper(double pos) const;
        QVector<ColorStop> d_stops;
        bool d_doAlpha;
    };

private:
    ColorStops d_colorStops;
    Mode d_mode;
};
