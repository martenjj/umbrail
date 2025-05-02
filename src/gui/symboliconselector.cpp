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

#include "symboliconselector.h"

#include <qlistwidget.h>
#include <qcombobox.h>
#include <qformlayout.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qdebug.h>
#include <qguiapplication.h>
#include <qelapsedtimer.h>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <klistwidgetsearchline.h>

#include "abstracticonprovider.h"
#include "settings.h"


#define MAX_RECENT		50			// maximum size of history


SymbolIconSelector::SymbolIconSelector(const QString &sym, PointIcon::IconNamespace nsp, QWidget *pnt)
    : DialogBase(pnt),
      DialogStateSaver(this)
{
    mSelectedName = sym;
    mInitialNamespace = PointIcon::NamespaceAuto;

    setObjectName("SymbolIconSelector");
    setWindowTitle(i18n("Select Symbol"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Reset);
    setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Select"));
    setButtonText(QDialogButtonBox::Reset, i18nc("@action:button", "Clear"));

    QWidget *w = new QWidget(this);
    QFormLayout *fl = new QFormLayout(w);

    mSourceCombo = new QComboBox(this);
    mSourceCombo->setSizePolicy(QSizePolicy::Expanding, mSourceCombo->sizePolicy().verticalPolicy());
    mSourceCombo->addItem(QIcon::fromTheme("view-history"), i18n("Recent"), PointIcon::NamespaceAuto);
    fl->addRow(i18n("Symbol set:"), mSourceCombo);

    const auto *providers = PointIcon::allProviders();
    for (const AbstractIconProvider *provider : std::as_const(*providers))
    {
        const QString iconName = QString("logo-")+provider->internalName();
        mSourceCombo->addItem(QIcon::fromTheme(iconName), provider->displayName(), provider->namespaceId());
    }

    // If no explicit icon namespace is specified, then see whether any
    // of the icon providers recognise the name.  If so then use that
    // provider's namespace.
    if (nsp==PointIcon::NamespaceAuto && !sym.isEmpty())
    {
        const PointIcon *pi = PointIcon::create(sym, nsp);
        nsp = pi->nsp();
    }

    // Just note the initial namespace for now;  it will be checked and
    // used to set up the GUI options in restoreConfig().
    mInitialNamespace = nsp;

    mSearchLine = new KListWidgetSearchLine(this);
    mSearchLine->setPlaceholderText(i18n("Symbol name..."));
    fl->addRow(i18n("Filter:"), mSearchLine);

    fl->addItem(DialogBase::verticalSpacerItem());

    mList = new QListWidget(this);
    mList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mList->setSelectionMode(QAbstractItemView::SingleSelection);
    mList->setViewMode(QListView::IconMode);
    mList->setMovement(QListView::Static);
    mList->setUniformItemSizes(true);
    mList->setSpacing(DialogBase::verticalSpacing());
    mList->setResizeMode(QListView::Adjust);
    mList->setMinimumWidth(400);
    fl->addRow(mList);
    // The list will be generated for the selected symbol set
    // in restoreConfig() below.

    mSearchLine->setListWidget(mList);
    setMainWidget(w);
    setStateSaver(this);

    connect(mSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SymbolIconSelector::slotSourceChanged);
    connect(mList, &QListWidget::itemSelectionChanged, this, &SymbolIconSelector::slotSelectionChanged);
    connect(buttonBox()->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &SymbolIconSelector::slotClearIcon);
    connect(mSearchLine, &QLineEdit::textChanged, this, &SymbolIconSelector::slotSelectionChanged);
}


SymbolIconSelector::~SymbolIconSelector()
{
    disconnect(mSearchLine, nullptr, nullptr, nullptr);
}


QString SymbolIconSelector::selectedIconName() const
{
    QList<QListWidgetItem *> sel = mList->selectedItems();
    if (sel.isEmpty()) return (QString());
    QListWidgetItem *item = sel.first();
    return (item->text());
}


PointIcon::IconNamespace SymbolIconSelector::selectedNamespace() const
{
    // If a real symbol set is selected in the source combo box,
    // then use that.
    PointIcon::IconNamespace nsp = static_cast<PointIcon::IconNamespace>(mSourceCombo->currentData().toInt());
    if (nsp!=PointIcon::NamespaceAuto) return (nsp);

    // If displaying the "Recent" symbols, then use the namespace ID
    // that was set for the selected symbol item by slotSourceChanged().
    QList<QListWidgetItem *> sel = mList->selectedItems();
    if (sel.isEmpty()) return (nsp);
    QListWidgetItem *item = sel.first();
    return (static_cast<PointIcon::IconNamespace>(item->data(Qt::UserRole).toInt()));
}


void SymbolIconSelector::slotSourceChanged()
{
    const PointIcon::IconNamespace nsp = static_cast<PointIcon::IconNamespace>(mSourceCombo->currentData().toInt());
    const bool isRecent = (nsp==PointIcon::NamespaceAuto);

    // This may take some time for OsmAnd...
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QElapsedTimer timer;
    timer.start();

    mList->clear();
    QListWidgetItem *selectedItem = nullptr;

    QStringList names = (isRecent ? mRecent : PointIcon::allNames(nsp));
    // Hopefully more efficient to sort the names before creating
    // the list view, instead of sorting the view items afterwards.
    // The recent history is not sorted but is left with the most
    // recently used at the start, as set by saveConfig().
    if (!isRecent) std::sort(names.begin(), names.end());

    for (const QString &name : std::as_const(names))
    {
        QString thisName = name;
        PointIcon::IconNamespace thisNsp = nsp;
        QString thisTip = thisName;

        if (isRecent)
        {
            // The name from the saved history is in the form "name:symset".
            // Split it into those two parts and format them for display.
            const int idx = name.indexOf(':');
            if (idx!=-1)
            {
                thisName = name.left(idx);
                thisNsp = PointIcon::namespaceId(name.mid(idx+1).toLatin1());
                thisTip = thisName+" ("+PointIcon::namespaceDisplayName(thisNsp)+")";
            }
        }

        QListWidgetItem *item = new QListWidgetItem(PointIcon::create(thisName, thisNsp)->icon(), thisName);
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        // AutoToolTipDelegate does not work for this sort of view,
        // so unconditionally set the tool tip.
        item->setToolTip(thisTip);
        item->setSizeHint(QSize(80, 50));
        // This is needed to retrieve the namespace that a "Recent" icon belongs to.
        item->setData(Qt::UserRole, thisNsp);
        mList->addItem(item);

        if (!isRecent && name==mSelectedName) selectedItem = item;
    }

    if (selectedItem!=nullptr)
    {
        selectedItem->setSelected(true);
        mList->setCurrentItem(selectedItem);
        mList->scrollToItem(selectedItem, QAbstractItemView::PositionAtCenter);
    }

    QGuiApplication::restoreOverrideCursor();
    slotSelectionChanged();
    qDebug() << "display took" << (timer.nsecsElapsed()/1000000) << "ms for" << names.count() << "icons";
}


void SymbolIconSelector::slotSelectionChanged()
{
    // KListWidgetSearchLine works by hiding the items that do not
    // match the filter.  But the decision here needs to be made after
    // the filtering has actually been updated, which happens after a
    // 200ms delay set in KListWidgetSearchLinePrivate::_k_queueSearch().
    QTimer::singleShot(250, this, [this]()
    {
        QListWidgetItem *item = mList->currentItem();
        setButtonEnabled(QDialogButtonBox::Ok, item!=nullptr && !item->isHidden());
    });
}


void SymbolIconSelector::slotClearIcon()
{
    mList->setCurrentItem(nullptr);
    accept();
}


void SymbolIconSelector::saveConfig(QDialog *dialog, KConfigGroup &grp) const
{
    grp.writeEntry("SymbolSet", PointIcon::namespaceInternalName(static_cast<PointIcon::IconNamespace>(mSourceCombo->currentData().toInt())));

    DialogStateSaver::saveConfig(dialog, grp);

    QString name = selectedIconName();
    if (name.isEmpty()) return;
    name += ":"+PointIcon::namespaceInternalName(selectedNamespace());

    QStringList r = mRecent;				// need to update the list
    if (r.contains(name)) r.removeAll(name);		// remove any already there
    r.prepend(name);					// put most recent at front
    if (r.count()>MAX_RECENT) r.resize(MAX_RECENT);	// enforce the size limit
    Settings::setRecentIcons(r);
}


void SymbolIconSelector::restoreConfig(QDialog *dialog, const KConfigGroup &grp)
{
    DialogStateSaver::restoreConfig(dialog, grp);

    mRecent = Settings::recentIcons();
    QTimer::singleShot(0, this, &SymbolIconSelector::slotSourceChanged);

    PointIcon::IconNamespace nsp = PointIcon::NamespaceAuto;
    // If 'nsp' is NamespaceAuto this will return NULL.
    const AbstractIconProvider *provider = PointIcon::provider(nsp);

    if (provider!=nullptr && provider->isEnabled())
    {
        qDebug() << "using specified provider" << provider->internalName();
        nsp = provider->namespaceId();
    }
    else
    {
        const QByteArray nsn = grp.readEntry("SymbolSet", QByteArray());
        // If 'nsn' is a null or unrecognised string this will return NamespaceAuto
        nsp = PointIcon::namespaceId(nsn);
    }

    // If 'nsp' is NamespaceAuto this will return the index of "Recent"
    const int idx = mSourceCombo->findData(nsp);
    if (idx!=-1) mSourceCombo->setCurrentIndex(idx);
}
