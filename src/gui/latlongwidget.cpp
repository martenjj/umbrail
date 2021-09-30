
#include "latlongwidget.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qboxlayout.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <kcolorscheme.h>

#include <kfdialog/dialogbase.h>

#include "pluginmanager.h"
#include "abstractcoordinatehandler.h"


LatLongWidget::LatLongWidget(QWidget *pnt)
    : QFrame(pnt)
{
    setObjectName("LatLongWidget");

    // Tab container
    mTabs = new QTabWidget(this);
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setMargin(0);
    hb->addWidget(mTabs);

    KColorScheme sch(QPalette::Normal);

    // Coordinate system tabs
    const QList<QObject *> plugins = PluginManager::self()->loadPlugins(PluginManager::CoordinatePlugin);
    qDebug() << "have" << plugins.count() << "coordinate plugins";
    foreach (QObject *obj, plugins)
    {
        AbstractCoordinateHandler *handler = qobject_cast<AbstractCoordinateHandler *>(obj);
        if (handler==nullptr)				// should never happen
        {
            qWarning() << "Null plugin!";
            continue;
        }

        handler->setParent(this);			// destroy when we're finished

        QWidget *w = new QWidget(this);			// top level widget for tab
        QVBoxLayout *vbl = new QVBoxLayout(w);		// layout for that

        QWidget *hw = handler->createWidget(w);		// get handler to create widget
        vbl->addWidget(hw);				// at top of tab widget
        vbl->addStretch(1);				// stretch in middle

        QLabel *lab = new QLabel(i18n("(Message)"), w);	// message label at bottom
        QPalette pal = lab->palette();
        pal.setColor(QPalette::WindowText, sch.foreground(KColorScheme::NegativeText).color());
        lab->setPalette(pal);				// set error message colour
        vbl->addWidget(lab);

        connect(handler, &AbstractCoordinateHandler::valueChanged, this, &LatLongWidget::slotValueChanged);
        connect(handler, &AbstractCoordinateHandler::statusMessage, lab, &QLabel::setText);

        mHandlers.append(handler);
        mTabs->addTab(w, handler->tabName());
    }

    // "Paste" button
    QAction *act = KStandardAction::paste(this);
    QPushButton *pasteButton = new QPushButton(act->icon(), act->text(), this);
    connect(pasteButton, &QAbstractButton::clicked, this, &LatLongWidget::slotPasteCoordinates);
    hb->addWidget(pasteButton);

    KConfigGroup grp = KSharedConfig::openConfig()->group(objectName());
    int idx = grp.readEntry("Index", -1);
    if (idx!=-1) mTabs->setCurrentIndex(idx);
}


LatLongWidget::~LatLongWidget()
{
    KConfigGroup grp = KSharedConfig::openConfig()->group(objectName());
    grp.writeEntry("Index", mTabs->currentIndex());
}


void LatLongWidget::setLatLong(double lat, double lon)
{
    qDebug() << lat << lon;

    mLatitude = lat;
    mLongitude = lon;

    foreach (AbstractCoordinateHandler *handler, mHandlers)
    {
        handler->setLatLong(lat, lon);
    }
}


void LatLongWidget::slotValueChanged()
{
    AbstractCoordinateHandler *changedHandler = qobject_cast<AbstractCoordinateHandler *>(sender());
    if (changedHandler==nullptr || !mHandlers.contains(changedHandler))
    {
        qWarning() << "called by unknown handler" << sender();
        return;
    }

    mLatitude = changedHandler->getLatitude();
    mLongitude = changedHandler->getLongitude();

    foreach (AbstractCoordinateHandler *handler, mHandlers)
    {
        // apart from the one just changed
        if (handler!=changedHandler) handler->setLatLong(mLatitude, mLongitude);
    }

    textChanged();
}


void LatLongWidget::textChanged()
{
    const bool valid = hasAcceptableInput();
    emit positionValid(valid);
    if (valid) emit positionChanged(mLatitude, mLongitude);
}


bool LatLongWidget::hasAcceptableInput() const
{
    bool ok = true;					// assume so to start

    foreach (AbstractCoordinateHandler *handler, mHandlers)
    {
        if (!handler->hasAcceptableInput()) ok = false;
    }
    return (ok);
}


void LatLongWidget::slotPasteCoordinates()
{
    QString text = QApplication::clipboard()->text().simplified();
    qDebug() << text;
    if (text.isEmpty())					// nothing to paste
    {
        KMessageBox::sorry(this, i18n("Nothing (or not text) to paste"));
        return;
    }

    QRegExp rx1("^(\\d+\\.\\d+)\\D+(\\d+\\.\\d+)");
    if (text.contains(rx1))				// try match in decimal format
    {
        double lat = rx1.cap(1).toDouble();		// assume success, because
        double lon = rx1.cap(2).toDouble();		// of regexp match above
        setLatLong(lat, lon);
        textChanged();
        return;
    }

    QRegExp rx2("^(\\d+)\\D+(\\d+)\\D(\\d+(\\.\\d+))\\D*([NnSs])\\D+(\\d+)\\D+(\\d+)\\D(\\d+(\\.\\d+))\\D*([EeWw])");
    if (text.contains(rx2))				// try match in DMS format
    {
        int latD = rx2.cap(1).toInt();
        int latM = rx2.cap(2).toInt();
        double latS = rx2.cap(3).toDouble();
        QChar latSign = (rx2.cap(5).left(1).toUpper())[0];

        int lonD = rx2.cap(6).toInt();
        int lonM = rx2.cap(7).toInt();
        double lonS = rx2.cap(8).toDouble();
        QChar lonSign = (rx2.cap(10).left(1).toUpper())[0];

        double lat = latD+(latM/60.0)+(latS/3600.0);
        if (latSign=='S') lat = -lat;

        double lon = lonD+(lonM/60.0)+(lonS/3600.0);
        if (lonSign=='W') lon = -lon;

        setLatLong(lat, lon);
        textChanged();
        return;
    }

    KMessageBox::sorry(this, i18n("Coordinate format not recognised"));
}
