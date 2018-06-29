
#ifndef PLOTEDITWIDGET_H
#define PLOTEDITWIDGET_H

#include <qframe.h>
#include <qvector.h>


class QGridLayout;
class QSpinBox;


class PlotEditWidget : public QFrame
{
    Q_OBJECT

public:
    enum EntryType
    {
        Bearing,
        Range
    };

    explicit PlotEditWidget(PlotEditWidget::EntryType type, QWidget *parent = nullptr);
    virtual ~PlotEditWidget() = default;

    void setPlotData(const QString &newData);
    QString plotData() const;

protected slots:
    void slotAddRow();
    void slotRemoveRow();

private:
    int rowOfButton(QObject *send) const;
    void updateLayout();

private:
    PlotEditWidget::EntryType mType;
    QGridLayout *mLayout;
    QVector<QSpinBox *> mFields;
};


#endif							// PLOTEDITWIDGET_H
