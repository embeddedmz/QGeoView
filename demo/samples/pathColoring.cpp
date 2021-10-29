#include "samples/polyline.h"
#include "samples/placemark.h"

#include "samples/colorMap.h"
#include "samples/colorMapPresetDialog.h"
#include "samples/rescaleRangeDialog.h"

/*        QVector<QGV::GeoPos> linePoints{
        QGV::GeoPos{ 43.28849853885284, -0.40097961338582416 },
        QGV::GeoPos{ 43.288607663101814, -0.4011056068729572 },
        QGV::GeoPos{ 43.28870169558679, -0.4012124625167549 },
        QGV::GeoPos{ 43.288837520817815, -0.40126349922217774 },
        QGV::GeoPos{ 43.289008175284756, -0.40080258793938295 },
    };
    ui->geoMap->addItem(new Polyline(linePoints, Qt::GlobalColor::red));
    ui->geoMap->addItem(new Placemark(QGV::GeoPos(43.28885725761855, -0.40090465730287766)));

    // plus tard...
    //ui->geoMap->addWidget(new QGVWidgetColorBar());
    
    // see you later alligator !
    #if false
    LinearColorMap lcm = ColorMapPresets::controlPointsToLinearColorMap(ColorMapPresets::Jet());

    ColorMapPresetDialog dialog(this);
    dialog.setCurrentControlPoints(ColorMapPresets::Jet());
    // QObject::connect(&dialog, &ColorMapPresetDialog::presetApplied, [this]()
    //{ });
    dialog.exec();

    // retrieve the original range
    double rangeMin = 0;
    double rangeMax = 0;

    if (rangeMin == rangeMax) {
        ++rangeMax; // to avoid rescale failing
    }

    // show a dialog box
    RescaleRangeDialog rescaleDlg(this);
    rescaleDlg.setRange(rangeMin, rangeMax);
    if (rescaleDlg.exec() == QDialog::Accepted) {
        const double dialogMinRange = rescaleDlg.minimum();
        const double dialogMaxRange = rescaleDlg.maximum();
        if (dialogMinRange != rangeMin || dialogMaxRange != rangeMax) {
            // ....
        }
    }
    #endif
    
    */