
#ifndef EXPORTFILEDIALOGUE_H
#define EXPORTFILEDIALOGUE_H


#include <qabstractitemmodel.h>

#include <kfdialog/dialogbase.h>

#include "importerexporteroptions.h"

class QTreeView;
class QUrl;
class KUrlRequester;
class TrackDataItem;


class ExportFileDialogue : public DialogBase
{
    Q_OBJECT

public:
    explicit ExportFileDialogue(const QString &filter = QString(), QWidget *pnt = nullptr);
    virtual ~ExportFileDialogue() = default;

    void setSourceModel(QAbstractItemModel *model);

    QUrl selectedUrl() const;
    ImporterExporterOptions options() const;

private slots:
    void slotSettingChanged();

private:
    KUrlRequester *mUrlRequester;
    QTreeView *mListView;
};

#endif							// EXPORTFILEDIALOGUE_H
