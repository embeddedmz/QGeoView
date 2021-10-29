#include "colorMapPresetDialog.h"
#include "colorMapPresetToPixmap.h"

#include "ui_colorMapPresetDialog.h"

#include <QList>
#include <QPixmap>
#include <QPointer>
#include <QSize>
#include <QSortFilterProxyModel>
#include <QtDebug>

namespace
{
using namespace ColorMapPresets;

// Move to ColorMapsPresets ?
std::vector<ControlPoints> s_controlPointsCollection = { BlackBodyRadiation(),
                                                         CoolToWarm(),
                                                         Jet(),
                                                         Grayscale(),
                                                         XRay()
                                                       };
std::vector<std::string> s_controlPointsCollectionNames = { "Black Body Radiation",
                                                            "Cool to Warm",
                                                            "Jet",
                                                            "Grayscale",
                                                            "XRay"
                                                          };
}

class ColorMapPresetDialogTableModel : public QAbstractTableModel
{
    typedef QAbstractTableModel Superclass;

    ColorMapPresetToPixmap PixmapRenderer;

    // 'mutable' allows us to avoid having to pregenerate all the pixmaps.
    mutable QList<QPixmap> Pixmaps;

    const QPixmap& pixmap(int row) const
    {
        this->Pixmaps.reserve(row + 1);

        // grow Pixmaps if needed.
        for (int cc = this->Pixmaps.size(); cc <= row; cc++)
        {
            this->Pixmaps.push_back(QPixmap());
        }

        if (this->Pixmaps[row].isNull())
        {
            this->Pixmaps[row] =
                    this->PixmapRenderer.render(s_controlPointsCollection[row], QSize(180, 20));
        }
        return this->Pixmaps[row];
    }

public:
    ColorMapPresetDialogTableModel(QObject* parentObject) :
        Superclass(parentObject)
    {
        this->Pixmaps.reserve(s_controlPointsCollection.size());
    }

    QModelIndex indexFromControlPoints(const ControlPoints& cps) const
    {
        for (size_t cc = 0, max = s_controlPointsCollection.size(); cc < max; ++cc)
        {
            if (cps.size() == s_controlPointsCollection.size() && 
                std::equal(cps.begin(), cps.end(), s_controlPointsCollection[cc].begin()))
            {
                return this->index(cc, 0, QModelIndex());
            }
        }
        return QModelIndex();
    }

    int rowCount(const QModelIndex& idx) const override
    {
        return idx.isValid() ? 0 : static_cast<int>(s_controlPointsCollection.size());
    }

    // we have a single column
    int columnCount(const QModelIndex& /*parent*/) const override { return 1; }

    QVariant data(const QModelIndex& idx, int role) const override
    {
        if (!idx.isValid() || idx.model() != this)
        {
            return QVariant();
        }

        switch (role)
        {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
            case Qt::StatusTipRole:
            case Qt::EditRole:
                return QString::fromStdString(s_controlPointsCollectionNames[idx.row()]);

            case Qt::DecorationRole:
                return this->pixmap(idx.row());

            case Qt::UserRole:
                break;
        }
        return QVariant();
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        Q_UNUSED(section);

        if (orientation == Qt::Vertical)
        {
            return QVariant();
        }

        switch (role)
        {
            case Qt::DisplayRole:
                return "Color maps";
        }

        return QVariant();
    }

private:
    Q_DISABLE_COPY(ColorMapPresetDialogTableModel)

};

class ColorMapPresetDialogProxyModel : public QSortFilterProxyModel
{
    typedef QSortFilterProxyModel Superclass;
    ColorMapPresetDialog::Modes Mode;

public:
    ColorMapPresetDialogProxyModel(ColorMapPresetDialog::Modes mode, QObject* parentObject = nullptr) :
        Superclass(parentObject),
        Mode(mode)
    {
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        if (!this->Superclass::filterAcceptsRow(sourceRow, sourceParent))
        {
            return false;
        }

        QModelIndex idx = this->sourceModel()->index(sourceRow, 0, sourceParent);
        switch (this->Mode)
        {
            case ColorMapPresetDialog::SHOW_ALL:
                return true;

            case ColorMapPresetDialog::SHOW_INDEXED_COLORS_ONLY:
                return this->sourceModel()->data(idx, Qt::UserRole).toBool();

            case ColorMapPresetDialog::SHOW_NON_INDEXED_COLORS_ONLY:
                return !this->sourceModel()->data(idx, Qt::UserRole).toBool();
        }
        return false;
    }

private:
    Q_DISABLE_COPY(ColorMapPresetDialogProxyModel)
};

struct ColorMapPresetDialog::ColorMapPresetDialogInternals
{
    Ui::ColorMapPresetDialog Ui;
    QPointer<ColorMapPresetDialogTableModel> Model;
    QPointer<QSortFilterProxyModel> ProxyModel;

    ColorMapPresetDialogInternals(ColorMapPresetDialog::Modes mode, ColorMapPresetDialog* self) :
        Model(new ColorMapPresetDialogTableModel(self)),
        ProxyModel(new ColorMapPresetDialogProxyModel(mode, self))
    {
        this->Ui.setupUi(self);
        this->Ui.gridLayout->setVerticalSpacing(4);
        this->Ui.gridLayout->setHorizontalSpacing(4);
        this->Ui.verticalLayout->setSpacing(4);

        this->ProxyModel->setSourceModel(this->Model);
        this->ProxyModel->setFilterKeyColumn(0);
        this->ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

        this->Ui.gradients->setModel(this->ProxyModel);
    }
};

//-----------------------------------------------------------------------------
ColorMapPresetDialog::ColorMapPresetDialog(QWidget* parent /*= nullptr*/,
                                             Modes mode /*= SHOW_ALL*/) :
    Superclass(parent),
    Internals(new ColorMapPresetDialogInternals(mode, this))
{
    const Ui::ColorMapPresetDialog& ui = this->Internals->Ui;
    connect(ui.gradients->selectionModel(), &QItemSelectionModel::selectionChanged,
            this,                           &ColorMapPresetDialog::updateEnabledStateForSelection);

    updateEnabledStateForSelection();

    connect(ui.gradients, &QTableView::doubleClicked,
            this,         &ColorMapPresetDialog::triggerApply);
    connect(ui.apply,     &QPushButton::clicked,
            this,         &ColorMapPresetDialog::triggerApplyWithButton);
}

ColorMapPresetDialog::~ColorMapPresetDialog()
{
    delete Internals;
}

void ColorMapPresetDialog::setCurrentControlPoints(const ColorMapPresets::ControlPoints& ctrlPts)
{
    ColorMapPresetDialogInternals& internals = (*this->Internals);
    QModelIndex idx = internals.Model->indexFromControlPoints(ctrlPts);
    idx = internals.ProxyModel->mapFromSource(idx);
    if (idx.isValid())
    {
        internals.Ui.gradients->selectionModel()->setCurrentIndex(
                    idx, QItemSelectionModel::ClearAndSelect);
    }
}

const ColorMapPresets::ControlPoints& ColorMapPresetDialog::currentControlPoints()
{
    const ColorMapPresetDialogInternals& internals = *this->Internals;
    const Ui::ColorMapPresetDialog& ui = this->Internals->Ui;
    QModelIndex proxyIndex = ui.gradients->selectionModel()->currentIndex();
    if (proxyIndex.isValid())
    {
        QModelIndex idx = internals.ProxyModel->mapToSource(proxyIndex);
        return s_controlPointsCollection[idx.row()];
    }

    static ControlPoints emptyCtrlPts;
    return emptyCtrlPts;
}

void ColorMapPresetDialog::updateEnabledStateForSelection()
{
    const Ui::ColorMapPresetDialog& ui = this->Internals->Ui;
    QModelIndexList selectedRows = ui.gradients->selectionModel()->selectedRows();
    if (selectedRows.size() == 1)
    {
        this->updateForSelectedIndex(selectedRows[0]);
    }
    else
    {
        ui.apply->setEnabled(false);
    }
}

void ColorMapPresetDialog::updateForSelectedIndex(const QModelIndex& proxyIndex)
{
    (void) proxyIndex;

    // update "options" based on what's available.
    const ColorMapPresetDialogInternals& internals = *this->Internals;
    const Ui::ColorMapPresetDialog& ui = internals.Ui;

    //QModelIndex idx = internals.ProxyModel->mapToSource(proxyIndex);
    ui.apply->setEnabled(true);
}

void ColorMapPresetDialog::triggerApply(const QModelIndex& _proxyIndex)
{
    const ColorMapPresetDialogInternals& internals = *this->Internals;
    const Ui::ColorMapPresetDialog& ui = this->Internals->Ui;

    const QModelIndex proxyIndex =
            _proxyIndex.isValid() ? _proxyIndex : ui.gradients->selectionModel()->currentIndex();
    QModelIndex idx = internals.ProxyModel->mapToSource(proxyIndex);

    const auto& ctrlPts = s_controlPointsCollection[idx.row()];
    // TODO send it with the signal ?

    emit presetApplied(); // update the color space combo box with the preset's original one
}

void ColorMapPresetDialog::triggerApplyWithButton(const bool buttonClick)
{
    Q_UNUSED(buttonClick)

    triggerApply();
}
