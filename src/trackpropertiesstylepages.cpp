
#include "trackpropertiesstylepages.h"

#include <qformlayout.h>
#include <qcheckbox.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kcolorbutton.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "variableunitdisplay.h"
#include "style.h"
#include "mapview.h"







TrackItemStylePage::TrackItemStylePage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    kDebug();
    setObjectName("TrackItemStylePage");

    addSeparatorField();

    const TrackDataItem *item = items.first();
    const Style *s = item->style();
    kDebug() << "initial style" << *s;

    mLineColourButton = new KColorButton(MapView::resolveLineColour(item), this);
    mLineColourButton->setAlphaChannelEnabled(false);
    connect(mLineColourButton, SIGNAL(changed(const QColor &)), SLOT(slotColourChanged(const QColor &)));
    mFormLayout->addRow(i18n("Line colour:"), mLineColourButton);

    mLineInheritCheck = new QCheckBox(i18n("Inherit from parent"), this);
    mLineInheritCheck->setChecked(!s->hasLineColour());
    mFormLayout->addRow(QString::null, mLineInheritCheck);
}







void TrackItemStylePage::slotColourChanged(const QColor &col)
{
    mLineInheritCheck->setChecked(!col.isValid());
}












const Style TrackItemStylePage::newStyle() const
{
    if (mLineInheritCheck->isChecked()) return (Style::null);
    Style result;
    result.setLineColour(mLineColourButton->color());
    return (result);
}








TrackFileStylePage::TrackFileStylePage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    kDebug();
    setObjectName("TrackFileStylePage");

    mLineInheritCheck->setText(i18n("Use application default"));
}







TrackTrackStylePage::TrackTrackStylePage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    kDebug();
    setObjectName("TrackTrackStylePage");
}






TrackSegmentStylePage::TrackSegmentStylePage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    kDebug();
    setObjectName("TrackSegmentStylePage");
}






TrackPointStylePage::TrackPointStylePage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    kDebug();
    setObjectName("TrackPointStylePage");
}


CREATE_PROPERTIES_PAGE(File, Style);
CREATE_PROPERTIES_PAGE(Track, Style);
CREATE_PROPERTIES_PAGE(Segment, Style);
CREATE_PROPERTIES_PAGE(Point, Style);
