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

#include "trackpropertiesdialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qdebug.h>
#include <qtabwidget.h>
#include <qpushbutton.h>

#include <klocalizedstring.h>
#include <kiconloader.h>
#include <kconfiggroup.h>
#include <kstandardguiitem.h>

#include <kfdialog/dialogstatewatcher.h>

#include "trackdata.h"
#include "metadatamodel.h"
#include "dataindexer.h"
#include "trackpropertiespage.h"
#include "trackpropertiesgeneralpages.h"
#include "trackpropertiesdetailpages.h"
#include "trackpropertiesstylepages.h"
#include "trackpropertiesplotpages.h"
#include "trackpropertiesmetadatapages.h"


static int sNextPageIndex = -1;				// page index to open with


TrackPropertiesDialogue::TrackPropertiesDialogue(const QList<TrackDataItem *> *items, QWidget *pnt)
    : DialogBase(pnt),
      DialogStateSaver(this)
{
    setObjectName("TrackPropertiesDialogue");

    setWindowModality(Qt::WindowModal);
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Close);
    stateWatcher()->setSaveOnButton(buttonBox()->button(QDialogButtonBox::Close));
    mCloseButtonShown = true;

    Q_ASSERT(!items->isEmpty());
    const TrackDataItem *item = items->first();
    Q_ASSERT(item!=nullptr);
    mItemType = item->type();

    mDataModel = new MetadataModel(item, this);

    QWidget *w = new QWidget(this);
    QGridLayout *gl = new QGridLayout(w);
    gl->setColumnMinimumWidth(0, DialogBase::horizontalSpacing());
    gl->setColumnStretch(1, 1);
    gl->setColumnMinimumWidth(3, DialogBase::horizontalSpacing());

    QLabel *typeLabel = new QLabel("?", this);
    gl->addWidget(typeLabel, 0, 1, Qt::AlignLeft);

    // The icon for the item type that the properties are being shown for.
    //
    // Some of these are "user" icons, installed in the application's 'pics'
    // directory.  This is searched automatically by KIconLoader (and hence
    // QIcon::from Theme() also), but they are not loaded by requested size
    // in the same way as for installed icons.  Therefore we detect this
    // special case and, if the icon is a user icon, look to see if there is
    // a suitable size available.
    //
    // 16x16 icons are installed with the plain name, and 32x32 versions are
    // installed with a "-32" suffix.

    // TODO: if the items are files, there are more than one, and they are
    // of mixed types (although we only support GPX files at present), the
    // icon should be "unknown".
    QLabel *iconLabel = new QLabel(this);

    QIcon icon = item->icon();
    const QString iconName = icon.name();

    // For a coloured waypoint icon, iconName will be null.  The required
    // size has already been taken into account by WaypointImageProvider.
    if (!iconName.isEmpty())
    {
        // Use KIconLoader::User here to avoid an expensive search of all
        // installed icon themes.  If the icon is not a user icon then
        // we are not interested anyway.
        QString userPath = KIconLoader::global()->iconPath(iconName, KIconLoader::User, true);
        if (!userPath.isEmpty() && userPath.contains("/pics/"))
        {
            // This is a "user" icon.  Look for the 32x32 version (which should
            // always be present) and use it.
            userPath = KIconLoader::global()->iconPath((iconName+"-32"), KIconLoader::User, true);
            if (!userPath.isEmpty()) icon = QIcon(userPath);
            // QIcon::addFile(userPath, QSize(32, 32)) does not appear to work.
            else qWarning() << "32x32 icon for" << iconName << "not found";
        }
    }

    iconLabel->setPixmap(icon.pixmap(KIconLoader::SizeMedium));
    gl->addWidget(iconLabel, 0, 2, Qt::AlignRight);

    gl->setRowMinimumHeight(1, 2*DialogBase::verticalSpacing());

    mTabWidget = new QTabWidget(this);
    mTabWidget->setTabsClosable(false);
    gl->addWidget(mTabWidget, 2, 0, 1, -1);

    setMainWidget(w);

    const TrackPropertiesInterface *propif = dynamic_cast<const TrackPropertiesInterface *>(item);
    Q_ASSERT(propif!=nullptr);

    TrackPropertiesPage *page = propif->createPropertiesGeneralPage(items, this);
    addPage(page, i18nc("@title:tab", "General"));
    TrackItemGeneralPage *gp = qobject_cast<TrackItemGeneralPage *>(page);
    Q_ASSERT(gp!=nullptr);
    typeLabel->setText(gp->typeText(items->count()));

    page = propif->createPropertiesDetailPage(items, this);
    addPage(page, i18nc("@title:tab", "Details"));

    page = propif->createPropertiesStylePage(items, this);
    addPage(page, i18nc("@title:tab", "Style"), (items->count()==1));

    page = propif->createPropertiesPlotPage(items, this);
    addPage(page, i18nc("@title:tab", "Plot"), (items->count()==1));

    page = propif->createPropertiesMetadataPage(items, this);
    addPage(page, i18nc("@title:tab", "Metadata"), (items->count()==1));

    setButtonEnabled(QDialogButtonBox::Ok, false);	// no data changed yet

    setMinimumSize(320,380);
    setStateSaver(this);
							// now that everything is set up
    connect(mTabWidget, &QTabWidget::currentChanged, this, &TrackPropertiesDialogue::slotTabChanged);
    connect(mDataModel, &MetadataModel::metadataChanged, this, &TrackPropertiesDialogue::slotModelDataChanged);
}


void TrackPropertiesDialogue::addPage(TrackPropertiesPage *page,
                                      const QString &title,
                                      bool enabled)
{
    if (page!=nullptr)					// if page applies to item type
    {
        page->setDataModel(mDataModel);
        mTabWidget->addTab(page, title);
        mTabWidget->setTabEnabled(mTabWidget->count()-1, enabled);
    }
    else						// no page, just add a dummy one
    {
        QWidget *dummyPage = new QWidget(this);
        mTabWidget->addTab(dummyPage, title);
        mTabWidget->setTabEnabled(mTabWidget->count()-1, false);
    }

    mPageDataChanged.append(true);			// resize page update list
}							// and need refresh first time


void TrackPropertiesDialogue::slotModelDataChanged(int idx)
{
    qDebug() << DataIndexer::name(idx);

    bool ok = true;
    const int num = mTabWidget->count();
    for (int i = 0; i<num; ++i)
    {
        TrackPropertiesPage *page = qobject_cast<TrackPropertiesPage *>(mTabWidget->widget(i));
        if (page==nullptr) continue;			// cast will fail for dummy pages
        mPageDataChanged[i] = true;			// note change needed next show
        if (!page->isDataValid()) ok = false;		// check page data valid
    }

    setButtonEnabled(QDialogButtonBox::Ok, ok);
    setButtonGuiItem(QDialogButtonBox::Close, KStandardGuiItem::cancel());
    mCloseButtonShown = false;
}


static void blockChildSignals(const QObjectList &widgets, bool block)
{
    for (QObjectList::const_iterator it = widgets.begin(); it!=widgets.end(); ++it)
    {
        QWidget *w = qobject_cast<QWidget *>(*it);
        if (w!=nullptr) w->blockSignals(block);
    }
}


void TrackPropertiesDialogue::slotTabChanged(int idx)
{
    qDebug() << "tab" << idx << "changed?" << mPageDataChanged[idx];
    if (!mPageDataChanged[idx]) return;			// nothing to do

    TrackPropertiesPage *page = qobject_cast<TrackPropertiesPage *>(mTabWidget->widget(idx));
    if (page==nullptr) return;				// cast will fail for dummy pages

    // Block GUI widget change signals over the call to refreshData(),
    // so that they do not try to update the model again with the same data
    // that they have just read from it.  Just blocking 'page' signals will
    // not have the desired effect.
    //
    // See https://www.qtcentre.org/threads/37730-Blocksignals-for-all-widgets-within-a-QDialog

    const QObjectList &pageWidgets = page->children();
    blockChildSignals(pageWidgets, true);
    page->refreshData();				// update page display data
    blockChildSignals(pageWidgets, false);

    mPageDataChanged[idx] = false;			// note page now up to date
}


void TrackPropertiesDialogue::showEvent(QShowEvent *ev)
{
    DialogBase::showEvent(ev);				// restore size and shown tab
    slotTabChanged(mTabWidget->currentIndex());		// refresh tab now showing
}


void TrackPropertiesDialogue::saveConfig(QDialog *dialog, KConfigGroup &grp) const
{
    // Because of the setSaveOnButton(QDialogButtonBox::Close) used in
    // the constructor, this will be called when that button is used
    // regardless of whether it says "Close" or "Cancel".  So we use
    // the flag to check what the button says, and only save state
    // if it is still "Close".
    if (result()==QDialog::Rejected)			// "Close" or "Cancel" button
    {
        if (!mCloseButtonShown) return;			// button is currently "Cancel"
    }

    grp.writeEntry(QString("Index%1").arg(mItemType), mTabWidget->currentIndex());
    DialogStateSaver::saveConfig(dialog, grp);
}


void TrackPropertiesDialogue::restoreConfig(QDialog *dialog, const KConfigGroup &grp)
{
    int page = sNextPageIndex;				// page explicitly requested
    sNextPageIndex = -1;				// note as now used
    if (page==-1) page = grp.readEntry(QString("Index%1").arg(mItemType), 0);
							// if none set, get from config
    mTabWidget->setCurrentIndex(page);

    DialogStateSaver::restoreConfig(dialog, grp);
}


void TrackPropertiesDialogue::setNextPageIndex(int page)
{
    sNextPageIndex = page;				// page to open next time
}
