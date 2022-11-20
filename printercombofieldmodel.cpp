#include "printercombofieldmodel.h"


PrinterComboFieldModel::PrinterComboFieldModel(combo_list_field &c, QObject *parent)
    : QAbstractItemModel(parent), combo(&c)
{
}

QVariant PrinterComboFieldModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
    // FIXME: Implement me!
}

QModelIndex PrinterComboFieldModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row,column, parent))
    return createIndex(row, column, nullptr);
    else return QModelIndex();
    // FIXME: Implement me!
}

QModelIndex PrinterComboFieldModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
    // FIXME: Implement me!
}

int PrinterComboFieldModel::rowCount(const QModelIndex &parent) const
{
//    if (!parent.isValid())
//        return 0;
    return combo->get_combo().size();

}

int PrinterComboFieldModel::columnCount(const QModelIndex &parent) const
{
//    if (!parent.isValid())
//        return 0;
    return 1;
}

QVariant PrinterComboFieldModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole)
    return combo->get_combo_at(index.row()).c_str();
    else return QVariant();
}
