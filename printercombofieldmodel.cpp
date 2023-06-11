#include "printercombofieldmodel.h"


StringComboListModel::StringComboListModel(string_combo_list_field &c, QObject *parent)
    : QAbstractItemModel(parent), combo(&c)
{
}

QVariant StringComboListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
    // FIXME: Implement me!
}

QModelIndex StringComboListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row,column, parent))
    return createIndex(row, column, nullptr);
    else return QModelIndex();
    // FIXME: Implement me!
}

QModelIndex StringComboListModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
    // FIXME: Implement me!
}

int StringComboListModel::rowCount(const QModelIndex &parent) const
{
//    if (!parent.isValid())
//        return 0;
    return combo->get_list_of_str().size();

}

int StringComboListModel::columnCount(const QModelIndex &parent) const
{
//    if (!parent.isValid())
//        return 0;
    return 1;
}

QVariant StringComboListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole)
    return QString::fromLocal8Bit(combo->get_string_at(index.row()).c_str());
    else return QVariant();
}
