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

#include "trackpropertiesstylepages.h"

#include <qformlayout.h>
#include <qcheckbox.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kcolorbutton.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "variableunitdisplay.h"
#include "mapview.h"
#include "metadatamodel.h"
#include "dataindexer.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackItemStylePage							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackItemStylePage::TrackItemStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    qDebug();
    setObjectName("TrackItemStylePage");

    addSeparatorField();

    mLineColourButton = nullptr;			// no buttons created yet
    mLineInheritCheck = nullptr;
    mPointColourButton = nullptr;
    mPointInheritCheck = nullptr;

    const TrackDataItem *item = items->first();
    mIsTopLevel = (item->parent()==nullptr);

    const QVariant v = item->metadata("color");		// get old compatibility value
    if (!v.isNull())
    {
        qWarning() << "item" << item->name() << "uses old COLOR data";
        if (dynamic_cast<const TrackDataAbstractPoint *>(item)!=nullptr)
        {						// colour for a point
            dataModel()->setData(DataIndexer::index("pointcolor"), v);
            dataModel()->setData(DataIndexer::index("color"), QVariant());
        }
        else						// colour for a line/container
        {
            dataModel()->setData(DataIndexer::index("linecolor"), v);
            dataModel()->setData(DataIndexer::index("color"), QVariant());
        }
    }
}


static QString checkText(const QString &text, bool isTopLevel)
{
    // Use the explicit text for the button, if it is supplied.
    if (!text.isEmpty()) return (text);

    // Otherwise, choose the text depending on whether the reference item
    // is at the top level of the tree.
    return (isTopLevel ? i18n("Use application default") : i18n("Use colour from parent"));
}


void TrackItemStylePage::addLineColourButton(const QString &text)
{
    mLineColourButton = new KColorButton(this);
    mLineColourButton->setAlphaChannelEnabled(false);
    mLineColourButton->setEnabled(!isReadOnly());
    connect(mLineColourButton, &KColorButton::changed, this, &TrackItemStylePage::slotColourChanged);
    mFormLayout->addRow(i18n("Line colour:"), mLineColourButton);

    mLineInheritCheck = new QCheckBox(checkText(text, mIsTopLevel), this);
    mLineInheritCheck->setEnabled(!isReadOnly());
    connect(mLineInheritCheck, &QCheckBox::toggled, this, &TrackItemStylePage::slotInheritChanged);
    mFormLayout->addRow("", mLineInheritCheck);

    mLineColourButton->setProperty("isLine", true);	// identify which color/check set
    mLineInheritCheck->setProperty("isLine", true);
}


void TrackItemStylePage::addPointColourButton(const QString &text)
{
    mPointColourButton = new KColorButton(this);
    mPointColourButton->setAlphaChannelEnabled(false);
    mPointColourButton->setEnabled(!isReadOnly());
    connect(mPointColourButton, &KColorButton::changed, this, &TrackItemStylePage::slotColourChanged);
    mFormLayout->addRow(i18n("Point colour:"), mPointColourButton);

    mPointInheritCheck = new QCheckBox(checkText(text, mIsTopLevel), this);
    mPointInheritCheck->setEnabled(!isReadOnly());
    connect(mPointInheritCheck, &QCheckBox::toggled, this, &TrackItemStylePage::slotInheritChanged);
    mFormLayout->addRow("", mPointInheritCheck);

    mPointColourButton->setProperty("isLine", false);	// identify which color/check set
    mPointInheritCheck->setProperty("isLine", false);
}

//  The state of the colour selection - colour and inherit flag - needs to be
//  able to be stored in a single metadata item.  Since we do not support
//  alpha blending of item colours, the alpha component of the colour value
//  is used to encode the inherit flags, with 255 meaning a colour to be used
//  and 0 meaning inherit.  However, once resolved, only RGB colours are stored
//  in the item metadata and GPX files.
//
//  The inherit flag is encoded in that way, instead of setting the colour value
//  to an invalid QColor, so that the RGB value is not lost when the inherit flag
//  is toggled.

static inline const char *colourKey(bool isLine)
{
    return (isLine ? "linecolor" : "pointcolor");	// could use UK spelling, but
}							// do this for consistency


QColor TrackItemStylePage::getColourData(bool isLine)
{
    return (dataModel()->data(colourKey(isLine)).value<QColor>());
}


void TrackItemStylePage::setColourData(bool isLine, const QColor &col)
{
    dataModel()->setData(DataIndexer::index(colourKey(isLine)), col);
}


void TrackItemStylePage::slotColourChanged(const QColor &col)
{							// colour button selected colour
    const bool isLine = sender()->property("isLine").toBool();
    qDebug() << "line?" << isLine << "col" << col;

    // The KColorButton does not have its alpha channel enabled, so the
    // colour value passed in here will always be RGB only.  Selecting a
    // colour via a colour button always turns off the inherit flag.

    setColourData(isLine, col);

    if (isLine && mLineColourButton!=nullptr)
    {
        Q_ASSERT(mLineInheritCheck!=nullptr);
        mLineInheritCheck->setChecked(!col.isValid());
    }

    if (!isLine && mPointColourButton!=nullptr)
    {
        Q_ASSERT(mPointInheritCheck!=nullptr);
        mPointInheritCheck->setChecked(!col.isValid());
    }
}


void TrackItemStylePage::slotInheritChanged(bool on)
{							// inherit check box toggled
    const bool isLine = sender()->property("isLine").toBool();
    qDebug() << "line?" << isLine << "on" << on;

    QColor col = getColourData(isLine);
    col.setAlpha(on ? 0 : 255);
    qDebug() << "set col" << col << "valid?" << col.isValid();
    setColourData(isLine, col);
}


void TrackItemStylePage::setColourButtons(KColorButton *colBut, QCheckBox *inheritBut, bool isLine)
{
    const QColor col = getColourData(isLine);		// combined colour as stored
    const bool inherit = (col.alpha()==0);		// has it alpha component?
    const QColor rgbcol = QColor(col.rgb());		// remove any alpha component
    qDebug() << "line?" << isLine << "col" << col << "inherit?" << inherit;

    colBut->setColor(rgbcol);
    inheritBut->setChecked(inherit || !col.isValid());
}


void TrackItemStylePage::refreshData()
{
    if (mLineColourButton!=nullptr)
    {
        Q_ASSERT(mLineInheritCheck!=nullptr);
        setColourButtons(mLineColourButton, mLineInheritCheck, true);
    }

    if (mPointColourButton!=nullptr)
    {
        Q_ASSERT(mPointInheritCheck!=nullptr);
        setColourButtons(mPointColourButton, mPointInheritCheck, false);
    }
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackFileStylePage							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackFileStylePage::TrackFileStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    qDebug();
    setObjectName("TrackFileStylePage");

    addLineColourButton();
    addSeparatorField();
    addPointColourButton();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackTrackStylePage							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackTrackStylePage::TrackTrackStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    qDebug();
    setObjectName("TrackTrackStylePage");

    addLineColourButton();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackSegmentStylePage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackSegmentStylePage::TrackSegmentStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    qDebug();
    setObjectName("TrackSegmentStylePage");

    addLineColourButton();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackWaypointStylePage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackWaypointStylePage::TrackWaypointStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    qDebug();
    setObjectName("TrackWaypointStylePage");

    addPointColourButton(i18n("Use waypoint icon"));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackRouteStylePage							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackRouteStylePage::TrackRouteStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    qDebug();
    setObjectName("TrackRouteStylePage");

    addLineColourButton();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Page creation interface						//
//									//
//////////////////////////////////////////////////////////////////////////

CREATE_PROPERTIES_PAGE(File, Style)
CREATE_PROPERTIES_PAGE(Track, Style)
CREATE_PROPERTIES_PAGE(Segment, Style)
CREATE_PROPERTIES_PAGE(Waypoint, Style)
CREATE_PROPERTIES_PAGE(Route, Style)

NULL_PROPERTIES_PAGE(Trackpoint, Style)
NULL_PROPERTIES_PAGE(Folder, Style)
NULL_PROPERTIES_PAGE(Routepoint, Style)
