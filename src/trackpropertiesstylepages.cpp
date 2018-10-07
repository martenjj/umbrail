
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
        connect(mPointInheritCheck, &QCheckBox::toggled, this, &TrackItemStylePage::slotInheritChanged);
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
        connect(mLineInheritCheck, &QCheckBox::toggled, this, &TrackItemStylePage::slotInheritChanged);
        mFormLayout->addRow("", mLineInheritCheck);
    }
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

void TrackItemStylePage::slotColourChanged(const QColor &col)
{							// colour button selected colour
    // The KColorButton does not have its alpha channel enabled, so the
    // colour value passed in here will always be RGB only.  Selecting a
    // colour via a colour button always turns off the inherit flag.

    // TODO: this will need to handle 2 cases if File extended for both colours

    qDebug() << col;

    dataModel()->setData(DataIndexer::self()->index("color"), col);

    if (mLineColourButton!=nullptr)
    {
        Q_ASSERT(mLineInheritCheck!=nullptr);
        mLineInheritCheck->setChecked(!col.isValid());
    }

    if (mPointColourButton!=nullptr)
    {
        Q_ASSERT(mPointInheritCheck!=nullptr);
        mPointInheritCheck->setChecked(!col.isValid());
    }
}


void TrackItemStylePage::slotInheritChanged(bool on)
{							// inherit check box toggled
    // TODO: this will need to handle 2 cases if File extended for both colours
//     QCheckBox *check = qobject_cast<QCheckBox *>(sender());
//     Q_ASSERT(check!=nullptr);
//     if (check==mLineInheritCheck)			// for line colour
//     {
//         Q_ASSERT(mLineColourButton!=nullptr);

        QColor col = dataModel()->data("color").value<QColor>();
        col.setAlpha(on ? 0 : 255);
        qDebug() << "set col" << col << "valid?" << col.isValid();

        dataModel()->setData(DataIndexer::self()->index("color"), col);
//     }
//     else if (check==mPointInheritCheck)			// for point colour
//     {
//         Q_ASSERT(mPointColourButton!=nullptr);
// 
//     }
//     else Q_ASSERT(false);				// must be one or the other
}


void TrackItemStylePage::refreshData()
{
    qDebug();
    const QColor col = dataModel()->data("color").value<QColor>();
							// combined colour as stored
    const bool inherit = (col.alpha()==0);		// has it alpha component?
    const QColor rgbcol = QColor(col.rgb());		// remove any alpha component
    qDebug() << "col" << col << "inherit?" << inherit;

    if (mLineColourButton!=nullptr)
    {
        Q_ASSERT(mLineInheritCheck!=nullptr);
        mLineColourButton->setColor(rgbcol);
        mLineInheritCheck->setChecked(inherit || !col.isValid());
    }

    if (mPointColourButton!=nullptr)
    {
        Q_ASSERT(mPointInheritCheck!=nullptr);
        mPointColourButton->setColor(rgbcol);
        mPointInheritCheck->setChecked(inherit || !col.isValid());
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

    // TODO: use Item, detect whether ref item has parent
    if (mLineColourButton!=NULL) mLineInheritCheck->setText(i18n("Use application default"));
    if (mPointColourButton!=NULL) mPointInheritCheck->setText(i18n("Use application default"));
}


void TrackFileStylePage::refreshData()
{
    qDebug();
    TrackItemStylePage::refreshData();
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
    TrackItemStylePage::refreshData();
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
    TrackItemStylePage::refreshData();
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
    TrackItemStylePage::refreshData();
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
    TrackItemStylePage::refreshData();
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
