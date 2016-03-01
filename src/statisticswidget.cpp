
#include "statisticswidget.h"

#include <math.h>

#include <qgridlayout.h>
#include <qlabel.h>
#include <qprogressbar.h>

#include <kdebug.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#ifdef USE_KCOLORSCHEME
#include <kcolorscheme.h>
#endif

#include "mainwindow.h"
#include "filescontroller.h"
#include "filesview.h"
#include "trackdata.h"


StatisticsWidget::StatisticsWidget(QWidget *pnt)
    : KDialog(pnt),
      MainWindowInterface(pnt)
{
    kDebug();

    setObjectName("StatisticsWidget");
    setButtons(KDialog::Close);
    showButtonSeparator(true);

    mTotalPoints = 0;
    mWithTime = 0;
    mWithElevation = 0;
    mWithGpsSpeed = 0;
    mWithGpsHdop = 0;

    const QList<TrackDataItem *> items = filesController()->view()->selectedItems();
    for (int i = 0; i<items.count(); ++i) getPointData(items[i]);

    mWidget = new QWidget(this);
    mLayout = new QGridLayout(mWidget);

    addRow(i18nc("@title:row", "Total points:"), mTotalPoints, false);
    mLayout->setRowMinimumHeight(mLayout->rowCount(), KDialog::spacingHint());
    addRow(i18nc("@title:row", "With time:"), mWithTime);
    addRow(i18nc("@title:row", "With elevation:"), mWithElevation);
    addRow(i18nc("@title:row", "With GPS speed:"), mWithGpsSpeed);
    addRow(i18nc("@title:row", "With GPS HDOP:"), mWithGpsHdop);

    mLayout->setRowStretch(mLayout->rowCount(), 1);
    mLayout->setColumnStretch(5, 1);
    mLayout->setColumnMinimumWidth(2, KDialog::spacingHint());
    mLayout->setColumnMinimumWidth(4, KDialog::spacingHint());

    setMainWidget(mWidget);

    KConfigGroup grp = KGlobal::config()->group(objectName());
    restoreDialogSize(grp);
}


StatisticsWidget::~StatisticsWidget()
{
    KConfigGroup grp = KGlobal::config()->group(objectName());
    saveDialogSize(grp);
    kDebug() << "done";
}


void StatisticsWidget::addRow(const QString &text, int num, bool withPercent)
{
    const int row = mLayout->rowCount();

    QLabel *l = new QLabel(text, mWidget);
    mLayout->addWidget(l, row, 0, Qt::AlignRight);

    l = new QLabel(QString::number(num), mWidget);
    mLayout->addWidget(l, row, 1, Qt::AlignRight);

    if (withPercent)
    {
        const int pct = qRound(num*100/mTotalPoints);
        l = new QLabel(QString("(%1%)").arg(pct), mWidget);
        mLayout->addWidget(l, row, 3);

        QProgressBar *p = new QProgressBar(mWidget);
        p->setMinimum(0);
        p->setMaximum(100);
        p->setValue(pct);
        p->setMaximumHeight(p->height()/2);
        p->setTextVisible(false);

        QPalette pal = p->palette();
#ifdef USE_KCOLORSCHEME
        KColorScheme sch(QPalette::Normal);
        KColorScheme::BackgroundRole back = KColorScheme::NeutralBackground;
        if (pct<20) back = KColorScheme::NegativeBackground;
        else if (pct>80) back = KColorScheme::PositiveBackground;

        pal.setColor(QPalette::Normal, QPalette::Highlight, sch.background(back).color());
#else
        Qt::GlobalColor back;
        if (pct<20) back = Qt::darkGray;
        else if (pct<40) back = Qt::darkRed;
        else if (pct<60) back = Qt::red;
        else if (pct<80) back = Qt::darkYellow;
        else if (pct<90) back = Qt::darkGreen;
        else back = Qt::green;

        pal.setColor(QPalette::Normal, QPalette::Highlight, back);
        pal.setColor(QPalette::Inactive, QPalette::Highlight, back);
#endif
        p->setPalette(pal);
        mLayout->addWidget(p, row, 5);
    }
}


void StatisticsWidget::getPointData(const TrackDataItem *item)
{
    const TrackDataTrackpoint *tdp = dynamic_cast<const TrackDataTrackpoint *>(item);
    if (tdp!=NULL)					// is this a point?
    {
        ++mTotalPoints;					// count up total points

        const QDateTime dt = tdp->time();		// time available
        if (dt.isValid()) ++mWithTime;

        const double ele = tdp->elevation();		// elevation available
        if (!isnan(ele) && ele!=0) ++mWithElevation;	// (and also nozero)

        const QString speedMeta = tdp->metadata("speed");
        if (!speedMeta.isEmpty()) ++mWithGpsSpeed;	// GPS speed recorded

        const QString hdopMeta = tdp->metadata("hdop");
        if (!hdopMeta.isEmpty()) ++mWithGpsHdop;	// GPS HDOP recorded
    }
    else						// not a point, recurse for children
    {
        const int num = item->childCount(); 
        for (int i = 0; i<num; ++i) getPointData(item->childAt(i));
    }
}
