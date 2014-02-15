
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
    : QWidget(pnt)
{
    kDebug();
    setObjectName("TrackItemStylePage");
    Q_ASSERT(!items.isEmpty());

    mFormLayout = new QFormLayout(this);
    addSpacerField();

    const TrackDataDisplayable *item = dynamic_cast<const TrackDataDisplayable *>(items.first());
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




void TrackItemStylePage::slotDataChanged()
{
    emit enableButtonOk(isDataValid());
}





void TrackItemStylePage::slotColourChanged(const QColor &col)
{
    mLineInheritCheck->setChecked(!col.isValid());
}











bool TrackItemStylePage::isDataValid() const
{
    return (true);
}




const Style TrackItemStylePage::newStyle() const
{
    if (mLineInheritCheck->isChecked()) return (Style::null);
    Style result;
    result.setLineColour(mLineColourButton->color());
    return (result);
}





void TrackItemStylePage::addSpacerField()
{
    mFormLayout->addItem(new QSpacerItem(1, KDialog::spacingHint(), QSizePolicy::Minimum, QSizePolicy::Fixed));
}







TrackFileStylePage::TrackFileStylePage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemStylePage(items, pnt)
{
    kDebug();
    setObjectName("TrackFileStylePage");



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



QWidget *TrackDataFile::createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackFileStylePage(items, pnt));
}


QWidget *TrackDataTrack::createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackTrackStylePage(items, pnt));
}


QWidget *TrackDataSegment::createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackSegmentStylePage(items, pnt));
}


QWidget *TrackDataPoint::createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackPointStylePage(items, pnt));
}
