
#ifndef IMPORTFILEDIALOGUE_H
#define IMPORTFILEDIALOGUE_H


#include <kfdialog/dialogbase.h>

#include "importerbase.h"


class QCheckBox;
class KUrlRequester;


class ImportFileDialogue : public DialogBase
{
    Q_OBJECT

public:
    explicit ImportFileDialogue(const QString &filter = QString(), QWidget *pnt = nullptr);
    virtual ~ImportFileDialogue() = default;

    QUrl selectedUrl() const;
    void setOptions(ImporterExporterBase::Options opts);
    ImporterExporterBase::Options options() const;

private slots:
    void slotUrlChanged(const QString &text);

private:
    KUrlRequester *mUrlRequester;
    QCheckBox *mNoHomeCheck;
    QCheckBox *mMergeWaypointsCheck;
};


#endif							// IMPORTFILEDIALOGUE_H
