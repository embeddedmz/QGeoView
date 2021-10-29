#include "RescaleRangeDialog.h"

#include <algorithm>

#include <QDoubleValidator>

RescaleRangeDialog::RescaleRangeDialog(QWidget* widgetParent) :
    QDialog(widgetParent)
{
    m_Ui.setupUi(this);

    // Make sure the line edits only allow number inputs.
    QDoubleValidator* validator = new QDoubleValidator(this);
    m_Ui.MinimumScalar->setValidator(validator);
    m_Ui.MaximumScalar->setValidator(validator);

    // Connect the gui elements.
    connect(m_Ui.MinimumScalar,     &QLineEdit::textChanged, this, &RescaleRangeDialog::validate);
    connect(m_Ui.MaximumScalar,     &QLineEdit::textChanged, this, &RescaleRangeDialog::validate);

    connect(m_Ui.RescaleOnlyButton, &QPushButton::clicked,   this, &RescaleRangeDialog::accept);
    connect(m_Ui.CancelButton,      &QPushButton::clicked,   this, &RescaleRangeDialog::reject);
}

void RescaleRangeDialog::setRange(double min, double max)
{
    if (min > max)
    {
        std::swap(min, max);
    }

    // Update the displayed range.
    m_Ui.MinimumScalar->setText(QString::number(min, 'g', 6));
    m_Ui.MaximumScalar->setText(QString::number(max, 'g', 6));
}

double RescaleRangeDialog::minimum() const
{
    return m_Ui.MinimumScalar->text().toDouble();
}

double RescaleRangeDialog::maximum() const
{
    return m_Ui.MaximumScalar->text().toDouble();
}

void RescaleRangeDialog::validate()
{
    int dummy;
    QString tmp1 = m_Ui.MinimumScalar->text();
    QString tmp2 = m_Ui.MaximumScalar->text();

    if (m_Ui.MinimumScalar->validator()->validate(tmp1, dummy) == QValidator::Acceptable &&
        m_Ui.MaximumScalar->validator()->validate(tmp2, dummy) == QValidator::Acceptable &&
        tmp1.toDouble() < tmp2.toDouble())
    {
        m_Ui.RescaleOnlyButton->setEnabled(true);
    }
    else
    {
        m_Ui.RescaleOnlyButton->setEnabled(false);
    }
}
