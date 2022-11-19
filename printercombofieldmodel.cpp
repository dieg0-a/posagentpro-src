#include "printercombofieldmodel.h"


PrinterComboFieldModel::PrinterComboFieldModel(input_field &combo, QObject *parent)
    : QAbstractItemModel(parent), combo_list_field(&combo)
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
    return createIndex(row, column, &combo_list_field->get_combo_at(row));
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
    return combo_list_field->get_combo().size();

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
    return ((std::string *)index.internalPointer())->c_str();
    else return QVariant();
}
