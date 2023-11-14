
#include "mergepointsdialogue.h"

#include <qformlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include <klocalizedstring.h>

#include "pointiconprovider.h"
#include "categorieseditdialogue.h"
#include "listeditwidget.h"


MergePointsDialogue::MergePointsDialogue(QWidget *pnt)
    : DialogBase(pnt)
{
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    setButtonText(QDialogButtonBox::Ok, i18n("Merge"));
    setWindowTitle(i18n("Merge Points"));

    QWidget *w = new QWidget(this);
    QFormLayout *lay = new QFormLayout(w);

    mNameEdit = new QComboBox(w);
    mNameEdit->setEditable(true);
    mNameEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    connect(mNameEdit, &QComboBox::editTextChanged, this, &MergePointsDialogue::slotUpdateButtons);
    lay->addRow(i18n("Name:"), mNameEdit);

    mSymbolEdit = new QComboBox(w);
    mSymbolEdit->setEditable(false);
    mSymbolEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    lay->addRow(i18n("Symbol:"), mSymbolEdit);

    lay->addItem(DialogBase::verticalSpacerItem());

    mLatLongEdit = new QComboBox(w);
    mLatLongEdit->setEditable(false);
    mLatLongEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    lay->addRow(i18n("Position:"), mLatLongEdit);

    mElevationEdit = new QComboBox(w);
    mElevationEdit->setEditable(false);
    mElevationEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    lay->addRow(i18n("Elevation:"), mElevationEdit);

    lay->addItem(DialogBase::verticalSpacerItem());

    mAddressEdit = new QComboBox(w);
    mAddressEdit->setEditable(false);
    mAddressEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    lay->addRow(i18n("Address:"), mAddressEdit);

    lay->addItem(DialogBase::verticalSpacerItem());

    mCategoriesLabel = new ListEditWidget(this);
    connect(mCategoriesLabel, &ListEditWidget::editRequested, this, &MergePointsDialogue::slotEditCategories);
    lay->addRow(i18nc("@label:textbox", "Categories:"), mCategoriesLabel);

    // In NavMarks there was the facility to display and merge the source tags
    // (now using the metadata name "origin") here.  However, although
    // intended as a way to trace the origin of a waypoint (for which they
    // indeed do that), source tags have turned out to be not so useful in
    // any other way as to need a prominent place in the GUI.
    //
    // The original intention was that source tags would accumulate each time
    // a point was imported; however this led to long standing waypoints
    // having an ever growing list of tags as time went on.  So the logic was
    // changed to only retain the first tag found on a point, and assign it on
    // the first import, or when created any other way, for a new point.  This
    // then worked well as a record of the origin of collected points, although
    // the GUI for managing a point's source tags and managing the global list
    // was never really used.
    //
    // Therefore there is no GUI implemented here for merging the source
    // tags (or in the "Properties" dialogue for displaying or editing
    // them).  The only display of them is in the main points list and the
    // "Properties - Metadata" list.  They are merged automatically by this
    // manual merge operation in the same way as other metadata.

    // TODO: GUI for merge of type, time, status, description, colours

    setMainWidget(w);
    w->setMinimumWidth(500);
}


TrackDataWaypoint *MergePointsDialogue::resultPoint()
{
    // This should never happen, but if there is no valid input then return nothing.
    if (!buttonBox()->button(QDialogButtonBox::Ok)->isEnabled()) return (nullptr);

    TrackDataWaypoint *res = new TrackDataWaypoint;	// new merged result point

    res->setName(mNameEdit->currentText(), true);
    res->setMetadata("sym", mSymbolEdit->currentData());

    const QPointF p = mLatLongEdit->currentData().toPointF();
    res->setLatLong(p.x(), p.y());

    const double elev = mElevationEdit->currentData().toDouble();
    if (!ISNAN(elev)) res->setMetadata("ele", elev);

    // Merge all of the flags silently, except that the NewlyImported
    // flag is cleared unless it is set for all of the source points.
    // Combine all of the source tags silently.

    // TODO: merge other metadata that has no GUI

    TrackData::WaypointFlags f = TrackData::NewlyImported;
    QStringList combinedOrgs;
    for (const TrackDataWaypoint *tdw : qAsConst(*mPoints))
    {
        TrackData::WaypointFlags f1 = static_cast<TrackData::WaypointFlags>(tdw->metadata("flags").toInt());
        if (!(f1 & TrackData::NewlyImported)) f &= ~TrackData::NewlyImported;
        f |= (f1 & ~TrackData::NewlyImported);

        const QStringList &orgs = tdw->metadata("origin").toStringList();
        for (const QString &org : orgs)
        {
            if (!combinedOrgs.contains(org)) combinedOrgs.append(org);
        }
    }

    res->setMetadata("flags", static_cast<int>(f));
    res->setMetadata("origin", combinedOrgs);
    res->setMetadata("category", mCombinedCats);

    // Decomposing the address from its formatted display form into
    // individual components is not deterministic, because null
    // components are ignored by TrackData::formattedAddress().
    // Therefore copy the components individually from the selected
    // source point.
    const TrackDataWaypoint *tdw = mPoints->at(mAddressEdit->currentIndex());
    res->setMetadata("StreetAddress", tdw->metadata("StreetAddress"));
    res->setMetadata("City", tdw->metadata("City"));
    res->setMetadata("State", tdw->metadata("State"));
    res->setMetadata("PostalCode", tdw->metadata("PostalCode"));
    res->setMetadata("Country", tdw->metadata("Country"));

    return (res);					// caller takes ownership
}


static void disableSingleValueCombo(QComboBox *cb)
{
    const int num = cb->count();			// how many entries in combo
    if (num==0) return;					// nothing to look at

    bool allSame = true;				// assume so to start
    const QString first = cb->itemText(0);		// text of first item
    for (int idx = 1; idx<num; ++idx)			// look at all the others
    {
        if (cb->itemText(idx)!=first)			// not the same as first
        {
            allSame = false;				// some are different
            break;					// no need to look at more
        }
    }

    cb->setEnabled(!allSame);				// enable combo accordingly
}


void MergePointsDialogue::setPoints(const QList<const TrackDataWaypoint *> *points)
{
    mPoints = points;					// remember input list

    mNameEdit->clear();					// reset accumulating fields
    mCombinedCats.clear();

    int elevToSelect = -1;				// index to initially select
    int addrToSelect = -1;

    int idx = 0;					// current combo box index
    for (const TrackDataWaypoint *tdw : *mPoints)
    {
        // Name - editable combo box with the alternatives
        mNameEdit->addItem(tdw->name());

        // Symbol - non-editable combo box with the alternatives
        const QVariant v = tdw->metadata("sym");
        if (!v.isNull())
        {
            const QString &sym = v.toString();
            const PointIcon *pi = PointIconProvider::self()->icon(sym);
            mSymbolEdit->addItem(pi->icon(), sym, sym);
            // TODO: may need to show namespace
            //PointIcon::namespaceName(pi->nsp())
        }
        else mSymbolEdit->addItem(QIcon("unknown"), i18n("(none)"));

        // Latitude/Longtitude - non-editable combo box with the alternatives
        const double lat = tdw->latitude();
        const double lon = tdw->longitude();
        // Combine the numerical values into a QPointF so that they can be
        // stored in a QVariant - easier than using a QPair and having to
        // declare that as a QMetaType.  It doesn't matter that the X and Y
        // coordinates are reversed.
        mLatLongEdit->addItem(TrackData::formattedLatLong(lat, lon), QPointF(lat, lon));

        // Elevation - non-editable combo box with the alternatives
        double elev = tdw->elevation();
        if (!ISNAN(elev))
        {
            if (elevToSelect==-1) elevToSelect = idx;
            mElevationEdit->addItem(QString::number(elev, 'f', 1), elev);
        }
        else mElevationEdit->addItem(i18n("(none)"), NAN);

        // Categories - pre-merge the lists
        const QStringList cats = tdw->metadata("category").toStringList();
        for (const QString &cat : qAsConst(cats))
        {
            if (!mCombinedCats.contains(cat)) mCombinedCats.append(cat);
        }

        // Address - non-editable combo box with the alternatives
        QString addr = tdw->formattedAddress().join(", ");
        if (!addr.isEmpty() && addrToSelect==-1) addrToSelect = idx;
        mAddressEdit->addItem((!addr.isEmpty() ? addr : i18n("(none)")));

        ++idx;
    }

    mNameEdit->setCurrentIndex(0);			// initially select the first
    mLatLongEdit->setCurrentIndex(0);

    if (elevToSelect==-1) elevToSelect = 0;		// select the first reasonable
    mElevationEdit->setCurrentIndex(elevToSelect);
    if (addrToSelect==-1) addrToSelect = 0;
    mAddressEdit->setCurrentIndex(addrToSelect);

    mCategoriesLabel->setList(mCombinedCats);		// set from combined list

    // Diable non-editable combo boxes if all of the values are the same.
    // This indicates to the user that there is no choice needing to be made.
    disableSingleValueCombo(mSymbolEdit);
    disableSingleValueCombo(mLatLongEdit);
    disableSingleValueCombo(mElevationEdit);
    disableSingleValueCombo(mAddressEdit);

    slotUpdateButtons();
}


void MergePointsDialogue::slotUpdateButtons()
{
    setButtonEnabled(QDialogButtonBox::Ok, !mNameEdit->currentText().isEmpty());
}


void MergePointsDialogue::slotEditCategories()
{
    const TrackDataFile *root = mPoints->first()->root();
    Q_ASSERT(root!=nullptr);

    // The dialogue below is able to handle the case where
    // no categories have ever been created or imported
    // and therefore root-><categories() is NULL.
    CategoriesEditDialogue d(&mCombinedCats, root->categories(), this);
    if (!d.exec()) return;

    mCombinedCats = d.categories();
    mCategoriesLabel->setList(mCombinedCats);
}
