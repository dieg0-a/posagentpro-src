#ifndef PRINTERCOMBOFIELDMODEL_H
#define PRINTERCOMBOFIELDMODEL_H

#include <QAbstractItemModel>
#include "printer.hpp"

class PrinterComboFieldModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit PrinterComboFieldModel(combo_list_field &combo, QObject *parent = nullptr);
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    combo_list_field *combo;

private:
};

#endif // PRINTERCOMBOFIELDMODEL_H
