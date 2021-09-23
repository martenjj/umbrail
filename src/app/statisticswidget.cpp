
#include "statisticswidget.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qpushbutton.h>

#include <klocalizedstring.h>
#ifdef USE_KCOLORSCHEME
#include <kcolorscheme.h>
#endif

#include <kfdialog/dialogstatewatcher.h>

#include "mainwindow.h"
#include "filescontroller.h"
#include "filesview.h"
#include "trackdata.h"


StatisticsWidget::StatisticsWidget(QWidget *pnt)
    : DialogBase(pnt),
      ApplicationDataInterface(pnt)
{
    setObjectName("StatisticsWidget");
    setButtons(QDialogButtonBox::Close);

    mTotalPoints = 0;
    mWithTime = 0;
    mWithElevation = 0;
    mWithGpsSpeed = 0;
    mWithGpsHdop = 0;
    mWithGpsHeading = 0;

    QVector<const TrackDataAbstractPoint *> points;
    filesController()->view()->selectedPoints().swap(points);
    for (const TrackDataAbstractPoint *tdp : qAsConst(points)) getPointData(tdp);

    mWidget = new QWidget(this);
    mLayout = new QGridLayout(mWidget);

    addRow(i18nc("@title:row", "Total points:"), mTotalPoints, false);
    mLayout->setRowMinimumHeight(mLayout->rowCount(), DialogBase::verticalSpacing());
    addRow(i18nc("@title:row", "With time:"), mWithTime);
    addRow(i18nc("@title:row", "With elevation:"), mWithElevation);
    addRow(i18nc("@title:row", "With GPS speed:"), mWithGpsSpeed);
    addRow(i18nc("@title:row", "With GPS HDOP:"), mWithGpsHdop);
    addRow(i18nc("@title:row", "With GPS heading:"), mWithGpsHeading);

    mLayout->setRowStretch(mLayout->rowCount(), 1);
    mLayout->setColumnStretch(5, 1);
    mLayout->setColumnMinimumWidth(2, DialogBase::horizontalSpacing());
    mLayout->setColumnMinimumWidth(4, DialogBase::horizontalSpacing());

    setMainWidget(mWidget);
    stateWatcher()->setSaveOnButton(buttonBox()->button(QDialogButtonBox::Close));
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
        const int pct = qRound(num*100.0/mTotalPoints);
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


void StatisticsWidget::getPointData(const TrackDataAbstractPoint *point)
{
    const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(point);
    if (tdp!=nullptr)					// is this a point?
    {
        ++mTotalPoints;					// count up total points

        const QDateTime dt = tdp->time();		// time available
        if (dt.isValid()) ++mWithTime;

        const double ele = tdp->elevation();		// elevation available
        if (!ISNAN(ele) && ele!=0) ++mWithElevation;	// (and also nozero)

        const QVariant speedMeta = tdp->metadata("speed");
        if (!speedMeta.isNull()) ++mWithGpsSpeed;	// GPS speed recorded

        const QVariant hdopMeta = tdp->metadata("hdop");
        if (!hdopMeta.isNull()) ++mWithGpsHdop;		// GPS HDOP recorded

        const QVariant headingMeta = tdp->metadata("heading");
        if (!headingMeta.isNull()) ++mWithGpsHeading;	// GPS heading recorded
    }
}
