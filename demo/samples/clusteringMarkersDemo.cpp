#include "clusteringMarkersDemo.h"

/*
    //QString url = R"(C:\\Users\\Amine Mzoughi\\Desktop\\blu-circle.png)";
    //QString url = R"(C:\\Users\\mmzoughi\\Pictures\\blu-circle.png)";
    //QPixmap pix(url);

    QPixmap pix(64, 64);
    QPainter paint(&pix);
    QPolygon triangle = QVector<QPoint>{ QPoint{ 0, 0 }, QPoint{ 32, 63 }, QPoint{ 63, 0 } };
    paint.setPen(QPen(QBrush(Qt::GlobalColor::red),
                         1,
                         Qt::PenStyle::SolidLine,
                         Qt::PenCapStyle::RoundCap,
                         Qt::PenJoinStyle::RoundJoin));
    paint.setBrush(QBrush(Qt::GlobalColor::red));
    paint.drawPolygon(triangle, Qt::WindingFill);
    paint.end();

PlacemarkSetLayer* myPOIs = new PlacemarkSetLayer();
    // faudra le faire avant d'ajouter les POIs ou bien il faudra passer la geomap au ctor
    // ou bien changer l'archi de la couche
    ui->geoMap->addItem(myPOIs);
    myPOIs->setClustering(true);
    myPOIs->setClusteringTreeDepth(20);
    //myPOIs->setClustering(false);
    myPOIs->setImage(pix);
    //myPOIs->add(QGV::GeoPos{ 0, 0 });
    myPOIs->add(QGV::GeoPos{ 43.28849853885284, -0.40097961338582416 });
    myPOIs->add(QGV::GeoPos{ 43.288607663101814, -0.4011056068729572 });
    myPOIs->add(QGV::GeoPos{ 43.28870169558679, -0.4012124625167549 });
    myPOIs->add(QGV::GeoPos{ 43.288837520817815, -0.40126349922217774 });
    myPOIs->add(QGV::GeoPos{ 43.289008175284756, -0.40080258793938295 });*/