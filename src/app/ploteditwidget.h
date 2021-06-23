
#ifndef PLOTEDITWIDGET_H
#define PLOTEDITWIDGET_H

#include <qframe.h>
#include <qvector.h>


class QGridLayout;
class QSpinBox;
class QTimer;


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

signals:
    void dataChanged();

private:
    QSpinBox *createSpinBox(PlotEditWidget::EntryType type);
    int rowOfButton(QObject *send) const;
    void updateLayout(bool focusLast = false);

private slots:
    void slotFocusLast();

private:
    PlotEditWidget::EntryType mType;
    QGridLayout *mLayout;
    QVector<QSpinBox *> mFields;
    QTimer *mDataChangedTimer;
};

#endif							// PLOTEDITWIDGET_H
