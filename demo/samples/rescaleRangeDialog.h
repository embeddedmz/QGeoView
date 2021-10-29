#ifndef RESCALERANGEDIALOG_H
#define RESCALERANGEDIALOG_H

#include <QDialog>

#include "ui_RescaleRangeDialog.h"

class RescaleRangeDialog : public QDialog
{
    Q_OBJECT
public:
    RescaleRangeDialog(QWidget* parent = 0);

    double minimum() const;
    double maximum() const;
    void setRange(double min, double max);

protected slots:
    void validate();

protected:
    Ui::RescaleRangeDialog m_Ui;

};

#endif
