#include "printerdriverlistmodel.h"
#include "messagesystem.h"

PrinterDriverListModel::PrinterDriverListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant PrinterDriverListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    return QVariant();
}

int PrinterDriverListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;
    return GlobalState::printerDrivers().size();
    // FIXME: Implement me!
}

QVariant PrinterDriverListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole)
        return GlobalState::printerDrivers().at(index.row()).c_str();

    // FIXME: Implement me!
    return QVariant();
}
