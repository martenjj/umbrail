//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

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
