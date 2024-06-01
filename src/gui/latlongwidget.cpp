//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#include "latlongwidget.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qboxlayout.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qregularexpression.h>
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
    hb->setContentsMargins(0, 0, 0, 0);
    hb->addWidget(mTabs);

    KColorScheme sch(QPalette::Normal);

    // Coordinate system tabs
    const QList<QObject *> plugins = PluginManager::self()->loadPlugins(PluginManager::CoordinatePlugin);
    qDebug() << "have" << plugins.count() << "coordinate plugins";
    for (QObject *obj : plugins)
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

    for (AbstractCoordinateHandler *handler : std::as_const(mHandlers)) handler->setLatLong(lat, lon);
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

    for (AbstractCoordinateHandler *handler : std::as_const(mHandlers))
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

    for (const AbstractCoordinateHandler *handler : std::as_const(mHandlers))
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
        KMessageBox::error(this, i18n("Nothing (or not text) to paste"));
        return;
    }

    const QRegularExpression rx1("^(\\d+\\.\\d+)\\D+(\\d+\\.\\d+)");
    const QRegularExpressionMatch match1 = rx1.match(text);
    if (match1.hasMatch())				// try match in decimal format
    {
        double lat = match1.captured(1).toDouble();	// assume success, because
        double lon = match1.captured(2).toDouble();	// of regexp match above
        setLatLong(lat, lon);
        textChanged();
        return;
    }

    const QRegularExpression rx2("^(\\d+)\\D+(\\d+)\\D(\\d+(\\.\\d+))\\D*([NnSs])\\D+(\\d+)\\D+(\\d+)\\D(\\d+(\\.\\d+))\\D*([EeWw])");
    const QRegularExpressionMatch match2 = rx2.match(text);
    if (match2.hasMatch())				// try match in DMS format
    {
        int latD = match2.captured(1).toInt();
        int latM = match2.captured(2).toInt();
        double latS = match2.captured(3).toDouble();
        QChar latSign = (match2.captured(5).left(1).toUpper())[0];

        int lonD = match2.captured(6).toInt();
        int lonM = match2.captured(7).toInt();
        double lonS = match2.captured(8).toDouble();
        QChar lonSign = (match2.captured(10).left(1).toUpper())[0];

        double lat = latD+(latM/60.0)+(latS/3600.0);
        if (latSign=='S') lat = -lat;

        double lon = lonD+(lonM/60.0)+(lonS/3600.0);
        if (lonSign=='W') lon = -lon;

        setLatLong(lat, lon);
        textChanged();
        return;
    }

    KMessageBox::error(this, i18n("Coordinate format not recognised"));
}
