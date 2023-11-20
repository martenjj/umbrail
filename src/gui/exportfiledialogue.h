
#ifndef EXPORTFILEDIALOGUE_H
#define EXPORTFILEDIALOGUE_H


#include <qabstractitemmodel.h>

#include <kfdialog/dialogbase.h>

class QTreeView;
class QUrl;
class KUrlRequester;
class TrackDataItem;

//////////////////////////////////////////////////////////////////////////
//									//
//  HomePointsModel							//
//									//
//////////////////////////////////////////////////////////////////////////

class HomePointsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    HomePointsModel(QObject *pnt = nullptr);
    virtual ~HomePointsModel() = default;

    void setSourceModel(QAbstractItemModel *srcModel);

    int rowCount(const QModelIndex &idx) const override				{ return (mPoints.count()); }
    QModelIndex index(int row, int col, const QModelIndex &pnt) const override	{ return (createIndex(row, col, row)); }
    QModelIndex parent(const QModelIndex &idx) const override			{ return (QModelIndex()); }

    int columnCount(const QModelIndex &idx) const override;
    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orient, int role) const override;

    QString homePoint() const;
    QString workPoint() const;

private slots:
    void slotRebuildPointsList();

private:
    void buildPointsList(const TrackDataItem *item);

private:
    QAbstractItemModel *mSourceModel;
    QVector<const TrackDataItem *> mPoints;
    int mHomeIndex;
    int mWorkIndex;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  ExportFileDialogue							//
//									//
//////////////////////////////////////////////////////////////////////////

class ExportFileDialogue : public DialogBase
{
    Q_OBJECT

public:
    explicit ExportFileDialogue(const QString &filter = QString(), QWidget *pnt = nullptr);
    virtual ~ExportFileDialogue() = default;

    void setSourceModel(QAbstractItemModel *model);

    QUrl selectedUrl() const;
    QString homePoint() const;
    QString workPoint() const;

private slots:
    void slotSettingChanged();

private:
    KUrlRequester *mUrlRequester;
    QTreeView *mListView;
};

#endif							// EXPORTFILEDIALOGUE_H
