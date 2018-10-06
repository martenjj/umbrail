
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

    mLineColourButton = NULL;
    mPointColourButton = NULL;

    const TrackDataItem *item = items->first();
    const QColor col = item->metadata("color").value<QColor>();
    qDebug() << "initial colour" << col;

    // TODO: a function to add each button, call as appropriate in subclasses
    // TODO: TrackDataFile should have both colours!
    if (dynamic_cast<const TrackDataAbstractPoint *>(item)!=NULL)
    {							// points have "Point colour"
        mPointColourButton = new KColorButton(MapView::resolvePointColour(item), this);
        mPointColourButton->setAlphaChannelEnabled(false);
        connect(mPointColourButton, SIGNAL(changed(const QColor &)), SLOT(slotColourChanged(const QColor &)));
        mFormLayout->addRow(i18n("Point colour:"), mPointColourButton);

        mPointInheritCheck = new QCheckBox(i18n("Inherit from parent"), this);
        mPointInheritCheck->setChecked(!col.isValid());
        mFormLayout->addRow("", mPointInheritCheck);
    }
    else						// others have "Line colour"
    {
        mLineColourButton = new KColorButton(MapView::resolveLineColour(item), this);
        mLineColourButton->setAlphaChannelEnabled(false);
        connect(mLineColourButton, SIGNAL(changed(const QColor &)), SLOT(slotColourChanged(const QColor &)));
        mFormLayout->addRow(i18n("Line colour:"), mLineColourButton);

        mLineInheritCheck = new QCheckBox(i18n("Inherit from parent"), this);
        mLineInheritCheck->setChecked(!col.isValid());
        mFormLayout->addRow("", mLineInheritCheck);
    }
}


void TrackItemStylePage::slotColourChanged(const QColor &col)
{
    if (mLineColourButton!=NULL) mLineInheritCheck->setChecked(!mLineColourButton->color().isValid());
    if (mPointColourButton!=NULL) mPointInheritCheck->setChecked(!mPointColourButton->color().isValid());
}


QColor TrackItemStylePage::newColour() const
{
    QColor result;
    if (mLineColourButton!=NULL && !mLineInheritCheck->isChecked()) result = mLineColourButton->color();
    if (mPointColourButton!=NULL && !mPointInheritCheck->isChecked()) result = mPointColourButton->color();
    return (result);
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

    if (mLineColourButton!=NULL) mLineInheritCheck->setText(i18n("Use application default"));
    if (mPointColourButton!=NULL) mPointInheritCheck->setText(i18n("Use application default"));
}


void TrackFileStylePage::refreshData()
{
    qDebug();
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
}


void TrackTrackStylePage::refreshData()
{
    qDebug();
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
}


void TrackSegmentStylePage::refreshData()
{
    qDebug();
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

    if (mPointColourButton!=NULL) mPointInheritCheck->setText(i18n("Use waypoint icon"));
}


void TrackWaypointStylePage::refreshData()
{
    qDebug();
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

    if (mLineColourButton!=NULL) mLineInheritCheck->setText(i18n("Use application default"));
}


void TrackRouteStylePage::refreshData()
{
    qDebug();
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
