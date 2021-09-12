/***************************************************************************
 * QGeoView is a Qt / C ++ widget for visualizing geographic data.
 * Copyright (C) 2018-2020 Andrey Yaroshenko.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see https://www.gnu.org/licenses.
 ****************************************************************************/

#include "raster.h"

// GDAL Libraries
#include <gdal.h>
#include <gdal_priv.h>

// Technique to have a static constructor like in C# to make GDAL global initialization
class GDALInitializer
{
public:
    static GDALInitializer& instance()
    {
        static GDALInitializer inst{};
        return inst;
    }

    GDALInitializer(GDALInitializer const&) = delete;
    GDALInitializer(GDALInitializer&&) = delete;

    GDALInitializer& operator=(GDALInitializer const&) = delete;
    GDALInitializer& operator=(GDALInitializer&&) = delete;

    ~GDALInitializer() = default;

private:
    GDALInitializer()
    {
        // Initialize GDAL
        GDALAllRegister();
    }
};

// PIMPL idiom : to avoid having GDAL include in the header file
struct Raster::Internals
{
    Internals()
        : mGDALHandle(GDALInitializer::instance())
    {

    }
    GDALInitializer& mGDALHandle;
};

Raster::Raster(const QString& tifFile/*const QGV::GeoPos& geoPos*/)
    : mInternals(new Raster::Internals)
{
    setFlag(QGV::ItemFlag::IgnoreScale);
    setFlag(QGV::ItemFlag::IgnoreAzimuth);
    setFlag(QGV::ItemFlag::Highlightable);
    setFlag(QGV::ItemFlag::HighlightCustom);
    setFlag(QGV::ItemFlag::Highlightable);
    setFlag(QGV::ItemFlag::Transformed);
    //setGeometry(geoPos, QSize(32, 32), QPoint(16, 32));
    //const QString url = "http://maps.google.com/mapfiles/kml/paddle/blu-circle.png";
    //load(url);

    // Load image
    GDALDataset* dataset = static_cast<GDALDataset*>(GDALOpen(tifFile.toLocal8Bit().data(), GA_ReadOnly));

    // Get raster image size
    int rows = dataset->GetRasterYSize();
    int cols = dataset->GetRasterXSize();
    int channels = dataset->GetRasterCount();

    std::vector<std::vector<uchar>> bandData(channels);
    for (auto& mat : bandData)
    {
        mat.resize(size_t(rows * cols));
    }
    std::vector<uchar> outputImage(size_t(4 * rows * cols));

    //QImage bandImage(band_data.data(), size_out, size_out, QImage::Format_Grayscale8);

    // Iterate over each channel
    for (int i = 1; i <= channels; ++i)
    {
        // Fetch the band
        GDALRasterBand* band = dataset->GetRasterBand(i);

        // Read the data
        band->RasterIO(GF_Read, 0, 0, cols, rows, bandData[size_t(i - 1)].data(),
            cols, rows, GDT_Byte, 0, 0);        
    }

    for (size_t i = 0, j = 0; i < outputImage.size(); i += 4, j += 1)
    {
        outputImage[i] = bandData[0][j];
        outputImage[i + 1] = bandData[1][j];
        outputImage[i + 2] = bandData[2][j];
        outputImage[i + 3] = bandData[3][j];
    }

    // Create the QImage - plus tard on utilsera un QPixmap, plus adapté pour les affichages
    QImage qtImage(outputImage.data(), cols, rows, QImage::Format_RGBA8888);
    //qtImage.save("gdal_first_test.png");
}

QTransform Raster::projTransform() const
{
    // Later : Transform data to web-mercator projection
    // input projection -> "EPSG:3857"
    // le faire ici ?

    return isFlag(QGV::ItemFlag::Highlighted) ? QGV::createTransfromScale(projAnchor(), 1.2) : QTransform();
}

void Raster::projOnFlags()
{
    setOpacity(isFlag(QGV::ItemFlag::Highlighted) ? 0.3 : 1.0);
}
