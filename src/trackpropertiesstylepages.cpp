
#include "trackpropertiesstylepages.h"

#include <qformlayout.h>
#include <qcheckbox.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kcolorbutton.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "variableunitdisplay.h"
#include "style.h"
#include "mapview.h"


TrackItemStylePage::TrackItemStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    qDebug();
    setObjectName("TrackItemStylePage");

    addSeparatorField();

    mLineColourButton = NULL;
    mPointColourButton = NULL;

    const TrackDataItem *item = items->first();
    const Style *s = item->style();
    qDebug() << "initial style" << *s;

    if (dynamic_cast<const TrackDataWaypoint *>(item)!=NULL)
    {							// waypoints have "Point colour"
        mPointColourButton = new KColorButton(MapView::resolvePointColour(item), this);
        mPointColourButton->setAlphaChannelEnabled(false);
        connect(mPointColourButton, SIGNAL(changed(const QColor &)), SLOT(slotColourChanged(const QColor &)));
        mFormLayout->addRow(i18n("Point colour:"), mPointColourButton);

        mPointInheritCheck = new QCheckBox(i18n("Inherit from parent"), this);
        mPointInheritCheck->setChecked(!s->hasPointColour());
        mFormLayout->addRow("", mPointInheritCheck);
    }
    else						// others have "Line colour"
    {
        mLineColourButton = new KColorButton(MapView::resolveLineColour(item), this);
        mLineColourButton->setAlphaChannelEnabled(false);
        connect(mLineColourButton, SIGNAL(changed(const QColor &)), SLOT(slotColourChanged(const QColor &)));
        mFormLayout->addRow(i18n("Line colour:"), mLineColourButton);

        mLineInheritCheck = new QCheckBox(i18n("Inherit from parent"), this);
        mLineInheritCheck->setChecked(!s->hasLineColour());
        mFormLayout->addRow("", mLineInheritCheck);
    }
}


void TrackItemStylePage::slotColourChanged(const QColor &col)
{
    if (mLineColourButton!=NULL) mLineInheritCheck->setChecked(!mLineColourButton->color().isValid());
    if (mPointColourButton!=NULL) mPointInheritCheck->setChecked(!mPointColourButton->color().isValid());
}


const Style TrackItemStylePage::newStyle() const
{
    Style result;
    if (mLineColourButton!=NULL && !mLineInheritCheck->isChecked()) result.setLineColour(mLineColourButton->color());
    if (mPointColourButton!=NULL && !mPointInheritCheck->isChecked()) result.setPointColour(mPointColourButton->color());
    return (result);
}


TrackFileStylePage::TrackFileStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    qDebug();
    setObjectName("TrackFileStylePage");

    if (mLineColourButton!=NULL) mLineInheritCheck->setText(i18n("Use application default"));
    if (mPointColourButton!=NULL) mPointInheritCheck->setText(i18n("Use application default"));
}







TrackTrackStylePage::TrackTrackStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    qDebug();
    setObjectName("TrackTrackStylePage");
}



TrackRouteStylePage::TrackRouteStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    qDebug();
    setObjectName("TrackRouteStylePage");
}



TrackSegmentStylePage::TrackSegmentStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    qDebug();
    setObjectName("TrackSegmentStylePage");
}



TrackWaypointStylePage::TrackWaypointStylePage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    qDebug();
    setObjectName("TrackWaypointStylePage");
}



CREATE_PROPERTIES_PAGE(File, Style)
CREATE_PROPERTIES_PAGE(Track, Style)
CREATE_PROPERTIES_PAGE(Route, Style)
CREATE_PROPERTIES_PAGE(Segment, Style)
CREATE_PROPERTIES_PAGE(Waypoint, Style)

NULL_PROPERTIES_PAGE(Trackpoint, Style)
NULL_PROPERTIES_PAGE(Folder, Style)
NULL_PROPERTIES_PAGE(Routepoint, Style)
