// -*-mode:c++ -*-

#ifndef METADATAMODEL_H
#define METADATAMODEL_H
 
#include <QAbstractTableModel>


class QTimeZone;
class TrackDataItem;


class MetadataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MetadataModel(const TrackDataItem *item, QObject *pnt = nullptr);
    virtual ~MetadataModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    const QVariant data(int idx) const;
    const QVariant data(const QString &nm) const;
    void setData(int idx, const QVariant &value);

    const QTimeZone *timeZone() const			{ return (mTimeZone); }
    double latitude() const;
    double longitude() const;

signals:
    void metadataChanged(int idx);

private:
    void resolveTimeZone();

private:
    QMap<int,QVariant> mItemData;

    QString mParentTimeZone;
    bool mUseParentTimeZone;
    QTimeZone *mTimeZone;
};
 
#endif							// METADATAMODEL_H
