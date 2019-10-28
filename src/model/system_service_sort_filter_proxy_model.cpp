#include "system_service_sort_filter_proxy_model.h"
#include "system_service_table_model.h"

SystemServiceSortFilterProxyModel::SystemServiceSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool SystemServiceSortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    QModelIndex index0 =
        sourceModel()->index(row, SystemServiceTableModel::kSystemServiceNameColumn, parent);
    QModelIndex index1 =
        sourceModel()->index(row, SystemServiceTableModel::kSystemServiceDescriptionColumn, parent);
    QModelIndex index2 =
        sourceModel()->index(row, SystemServiceTableModel::kSystemServicePIDColumn, parent);

    bool rc = false;
    if (index0.isValid())
        rc |= sourceModel()->data(index0).toString().contains(filterRegExp());
    if (index1.isValid())
        rc |= sourceModel()->data(index1).toString().contains(filterRegExp());
    if (index2.isValid())
        rc |= sourceModel()->data(index2).toString().contains(filterRegExp());

    return rc;
}

bool SystemServiceSortFilterProxyModel::filterAcceptsColumn(int column,
                                                            const QModelIndex &parent) const
{
    return QSortFilterProxyModel::filterAcceptsColumn(column, parent);
}

bool SystemServiceSortFilterProxyModel::lessThan(const QModelIndex &left,
                                                 const QModelIndex &right) const
{
    switch (sortColumn()) {
        case SystemServiceTableModel::kSystemServicePIDColumn:
            return left.data().toUInt() < right.data().toUInt();
        case SystemServiceTableModel::kSystemServiceNameColumn:
        case SystemServiceTableModel::kSystemServiceDescriptionColumn: {
            int rc = QString::compare(left.data().toString(), right.data().toString(),
                                      Qt::CaseInsensitive);
            return (rc < 0);
        }
        default:
            break;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}