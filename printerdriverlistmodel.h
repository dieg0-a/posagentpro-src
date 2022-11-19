#ifndef PRINTERDRIVERLISTMODEL_H
#define PRINTERDRIVERLISTMODEL_H

#include <QAbstractListModel>

class PrinterDriverListModel : public QAbstractListModel
{
    Q_OBJECT


public:
    explicit PrinterDriverListModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

#endif // PRINTERDRIVERLISTMODEL_H
