#pragma once

#include <QDialog>
#include <QModelIndex>
#include <QScopedPointer>

#include "colorMapPresets.h"

class QModelIndex;

/**
* ColorMapPresetDialog is the dialog used by to show the user with a choice of color
* maps/presets to choose from.
*/
class ColorMapPresetDialog : public QDialog
{
    Q_OBJECT
    typedef QDialog Superclass;

public:
    /**
     * Used to control what kinds of presets are shown in the dialog.
     * This merely affects the presets that are hidden from the view.
     */
    enum Modes
    {
        SHOW_ALL,
        SHOW_INDEXED_COLORS_ONLY, // indexed colors are not used for the moment
        SHOW_NON_INDEXED_COLORS_ONLY
    };

    explicit ColorMapPresetDialog(QWidget* parent = nullptr,
                                   Modes mode = SHOW_ALL);
    ~ColorMapPresetDialog() override;

    /**
     * Set the current color map by using its control points
     */
    void setCurrentControlPoints(const ColorMapPresets::ControlPoints& ctrlPts);

    /**
     * Return current preset, if any.
     */
    const ColorMapPresets::ControlPoints& currentControlPoints();

signals:
    void presetApplied();

protected slots:
    void updateEnabledStateForSelection();
    void updateForSelectedIndex(const QModelIndex& proxyIndex);
    void triggerApply(const QModelIndex& proxyIndex = QModelIndex());
    void triggerApplyWithButton(const bool buttonClick);

private:
    Q_DISABLE_COPY(ColorMapPresetDialog)

    struct ColorMapPresetDialogInternals;
    ColorMapPresetDialogInternals* const Internals;
};
