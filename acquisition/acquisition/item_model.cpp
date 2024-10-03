#include <acquisition/item_model.h>

// Returns the index of the item in the model specified by the given row, column and parent index.
QModelIndex ItemModel::index(int row, int column, const QModelIndex& parent) const
{
    return QModelIndex();
}

// Returns the parent of the model item with the given index. If the item has no parent, an invalid QModelIndex is returned
QModelIndex ItemModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

// Returns the number of rows under the given parent. When the parent is valid it means that rowCount is returning the number of children of parent.
int ItemModel::rowCount(const QModelIndex& parent) const
{
    return 0;
}

// Returns the number of columns for the children of the given parent.
int ItemModel::columnCount(const QModelIndex& parent) const
{
    return 0;
}

// Returns the data stored under the given role for the item referred to by the index.
QVariant ItemModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        break;
    case Qt::ToolTipRole:
        break;
    case Qt::WhatsThisRole:
        break;
    default:
        break;
    };
    return QVariant();
}

// Returns true if parent has any children; otherwise returns false.
bool ItemModel::hasChildren(const QModelIndex& parent) const
{
    return false;
}


// Inserts count rows into the model before the given row. Items in the new row will be children of the item represented by the parent model index.
bool ItemModel::insertRows(int row, int count, const QModelIndex& parent)
{
    return true;
}

// Removes count rows starting with the given row under parent parent from the model.
bool ItemModel::removeRows(int row, int count, const QModelIndex& parent)
{
    return true;
}